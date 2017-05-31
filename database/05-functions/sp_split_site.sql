create or replace function sp_split_site(
    _site_id site.id%type
)
returns void
as
$$
declare _name site.name%type;
declare _short_name site.short_name%type;
declare _geog site.geog%type;
declare _enabled site.enabled%type;
declare _tile_id text;
declare _satellite_id satellite.id%type;
declare _new_site_id site.id%type;
begin
    select name,
           short_name,
           geog,
           enabled
    into _name,
         _short_name,
         _geog,
         _enabled
    from site
    where site.id = _site_id;

    if not found then
        raise exception 'Could not find site id %', _site_id;
    end if;

    for _tile_id,
        _satellite_id
    in
        select tile_id, 1
        from sp_get_site_tiles(_site_id, 1)
        union all
        select tile_id, 2
        from sp_get_site_tiles(_site_id, 2)
    loop
        insert into site(name,
                         short_name,
                         geog,
                         enabled)
        values (
            _name || '_' || _tile_id,
            _short_name || '_' || lower(_tile_id),
            _geog,
            _enabled
        )
        returning id
        into _new_site_id;

        insert into site_tiles(site_id,
                               satellite_id,
                               tiles)
        values (
            _new_site_id,
            _satellite_id,
            array[_tile_id]
        );

        insert into season(site_id,
                           name,
                           start_date,
                           end_date,
                           mid_date,
                           enabled)
        select _new_site_id,
               name,
               start_date,
               end_date,
               mid_date,
               enabled
        from season
        where season.site_id = _site_id;
    end loop;
end;
$$
    language plpgsql volatile;
