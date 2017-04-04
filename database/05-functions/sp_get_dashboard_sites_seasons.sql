CREATE OR REPLACE FUNCTION sp_get_dashboard_sites_seasons(_site_id smallint DEFAULT NULL::smallint)
  RETURNS TABLE(
    site_id site.id%type,
    site_name site.name%type,
    site_short_name site.short_name%type,
    site_enabled site.enabled%type,
    season_id season.id%type,
    season_name season.name%type,
    season_start_date season.start_date%type,
    season_end_date season.end_date%type,
    season_mid_date season.mid_date%type,
    season_enabled season.enabled%type
) AS
$BODY$
BEGIN
    return query
        select site.id,
               site.name,
               site.short_name,
               site.enabled,
               season.id,
               season.name,
               season.start_date,
               season.end_date,
               season.mid_date,
               season.enabled
        from site
        left outer join season on season.site_id = site.id
        where _site_id is null or site.id = _site_id
        order by site.id, season.id;
END
$BODY$
  LANGUAGE plpgsql STABLE;
ALTER FUNCTION sp_get_dashboard_sites_seasons(smallint)
  OWNER TO admin;
