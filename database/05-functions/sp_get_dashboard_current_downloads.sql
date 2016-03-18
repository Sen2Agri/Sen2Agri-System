CREATE OR REPLACE FUNCTION sp_get_dashboard_current_downloads(IN _siteid smallint DEFAULT NULL::smallint)
  RETURNS TABLE(prod_name character varying, sat character varying, prod_date timestamp with time zone) AS
$BODY$
BEGIN
   RETURN QUERY
   
select product_name, satellite.satellite_name, product_date
from downloader_history
inner join satellite on downloader_history.satellite_id = satellite.id
inner join site on downloader_history.site_id = site.id AND  status_id =1 AND( $1 IS NULL OR site.id = _siteid);

END
$BODY$
  LANGUAGE plpgsql 