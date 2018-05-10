CREATE OR REPLACE FUNCTION sp_get_estimated_number_of_products(_siteid smallint DEFAULT NULL::smallint)
  RETURNS integer AS
$BODY$
DECLARE total_1 integer;
DECLARE total_2 integer;
DECLARE prod_hist integer;
BEGIN
	SELECT COALESCE(SUM(product_count),0)  into total_1 FROM downloader_count
	WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,max(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid ) AND satellite_id=1 GROUP BY site_id );	

	SELECT COALESCE(SUM(product_count),0)  into total_2 FROM downloader_count
	WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,max(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid) AND satellite_id=2 GROUP BY site_id );	
	-- RAISE NOTICE '%',total_1;
	-- RAISE NOTICE '%',total_2;

	SELECT count(dh.id) into  prod_hist
	FROM downloader_history dh
	WHERE (_siteid IS NULL OR site_id = _siteid) AND dh.created_timestamp >=  (Select max(last_updated) from downloader_count dc WHERE dc.site_id=dh.site_id );
		-- RAISE NOTICE '%',prod_hist;

	RETURN (total_1 + total_2) - prod_hist as nr;
END
$BODY$
  LANGUAGE plpgsql STABLE
