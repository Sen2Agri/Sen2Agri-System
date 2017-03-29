create or replace function sp_update_season(
    _id season.id%type,
    _site_id season.site_id%type,
    _name season.name%type,
    _start_date season.start_date%type,
    _end_date season.end_date%type,
    _mid_date season.mid_date%type,
    _enabled season.enabled%type
)
returns void as
$$
begin
    update season
    set site_id = coalesce(_site_id, site_id),
        name = coalesce(_name, name),
        start_date = coalesce(_start_date, start_date),
        end_date = coalesce(_end_date, end_date),
        mid_date = coalesce(_mid_date, mid_date),
        enabled = coalesce(_enabled, enabled)
    where id = _id;

    if not found then
        raise exception 'Invalid season % for site %', _name, _site_id;
    end if;
end;
$$
    language plpgsql volatile;
