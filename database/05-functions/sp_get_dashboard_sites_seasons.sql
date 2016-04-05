CREATE OR REPLACE FUNCTION sp_get_dashboard_sites_seasons(IN _site_id smallint DEFAULT NULL::smallint)
  RETURNS TABLE(id smallint, name character varying, short_name character varying, key character varying, value character varying) AS
$BODY$
BEGIN
   RETURN QUERY
   
    select site_id,results.name,results.short_name,results.key, results.value from (
    select type, site_id, site.name,site.short_name, cfg.key,cfg.value, row_number() over(partition by site_id, cfg.key order by type) as row
    from site
    inner JOIN lateral (
        SELECT 1 as type, config.site_id, config.key, config.value
        from config
        where config.site_id = site.id  and config.key like 'downloader.%season%'
        union all
        select 2 as type, site.id as site_id, config.key, config.value
        from config
        where config.site_id is null and config.key like 'downloader.%season%'
    ) cfg on site.id = cfg.site_id AND ($1 IS NULL OR site.id =$1)
) results
where row = 1;
 
END
$BODY$
  LANGUAGE plpgsql