-- Function: reports.sp_insert_l8_statistics()

-- DROP FUNCTION reports.sp_insert_l8_statistics();

CREATE OR REPLACE FUNCTION reports.sp_insert_l8_statistics()
  RETURNS TABLE(rows integer) AS
$BODY$

BEGIN
    RETURN QUERY
    WITH rows AS (
	INSERT INTO reports.l8_report(site_id, downloader_history_id, orbit_id, acquisition_date, product_name, status_description, status_reason, l2_product, clouds)
	SELECT (f).* FROM (SELECT reports.sp_get_l8_statistics(id) as f FROM public.site where enabled = true) from_site
	RETURNING 1)
	SELECT COUNT(*)::integer FROM rows;
END

$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION reports.sp_insert_l8_statistics()
  OWNER TO postgres;
