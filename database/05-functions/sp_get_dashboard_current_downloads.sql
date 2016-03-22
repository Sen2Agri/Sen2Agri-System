CREATE OR REPLACE FUNCTION sp_get_dashboard_current_downloads(IN _siteid smallint DEFAULT NULL::smallint)
  RETURNS TABLE(site character varying, prod_name character varying, sat character varying, prod_date timestamp with time zone) AS
$BODY$
BEGIN
   RETURN QUERY
   
SELECT site.name, product_name, satellite.satellite_name, product_date
FROM downloader_history
INNER JOIN satellite ON downloader_history.satellite_id = satellite.id
INNER JOIN site ON downloader_history.site_id = site.id AND  status_id =1 AND( $1 IS NULL OR site.id = _siteid)
ORDER BY site.name;

END
$BODY$
  LANGUAGE plpgsql