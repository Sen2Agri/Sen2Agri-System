create or replace function sp_get_site_seasons(
    _site_id site.id%type
)
returns table (
    id season.id%type,
    site_id season.site_id%type,
    name season.name%type,
    start_date season.start_date%type,
    end_date season.end_date%type,
    mid_date season.mid_date%type,
    enabled season.enabled%type
) as
$$
begin
    if _site_id is null then
        return query
            select
                season.id,
                season.site_id,
                season.name,
                season.start_date,
                season.end_date,
                season.mid_date,
                season.enabled
            from season
            order by season.site_id, season.start_date;
    else
        return query
            select
                season.id,
                season.site_id,
                season.name,
                season.start_date,
                season.end_date,
                season.mid_date,
                season.enabled
            from season
            where season.site_id = _site_id
            order by season.start_date;
    end if;
end;
$$
    language plpgsql stable;
