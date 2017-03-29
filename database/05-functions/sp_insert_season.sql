create or replace function sp_insert_season(
    _site_id season.site_id%type,
    _name season.name%type,
    _start_date season.start_date%type,
    _end_date season.end_date%type,
    _mid_date season.mid_date%type,
    _enabled season.enabled%type
)
returns season.id%type as
$$
declare _season_id season.id%type;
begin
    insert into season(
        site_id,
        name,
        start_date,
        end_date,
        mid_date,
        enabled
    ) values (
        _site_id,
        _name,
        _start_date,
        _end_date,
        _mid_date,
        _enabled
    )
    returning season.id
    into _season_id;

    perform sp_insert_default_scheduled_tasks(_site_id, _season_id);

    return _season_id;
end;
$$
    language plpgsql volatile;
