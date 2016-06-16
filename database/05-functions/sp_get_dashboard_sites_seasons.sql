CREATE OR REPLACE FUNCTION sp_get_dashboard_sites_seasons(IN _site_id smallint DEFAULT NULL::smallint)
  RETURNS TABLE(id smallint, name character varying, short_name character varying, summer_season_start text, summer_season_end text, winter_season_start text, winter_season_end text, enabled boolean) AS
$BODY$
BEGIN
    return query
        select site.id,
               site.name,
               site.short_name,
               max(case config.key when 'downloader.summer-season.start' then config.value end) as summer_season_start,
               max(case config.key when 'downloader.summer-season.end' then config.value end) as summer_season_end,
               max(case config.key when 'downloader.winter-season.start' then config.value end) as winter_season_start,
               max(case config.key when 'downloader.winter-season.end' then config.value end) as winter_season_start,
               site.enabled
        from site
        left outer join config on config.site_id = site.id
        where _site_id is null or site.id = _site_id
        group by site.id
        order by site.id;
END
$BODY$
  LANGUAGE plpgsql STABLE;
ALTER FUNCTION sp_get_dashboard_sites_seasons(smallint)
  OWNER TO admin;
