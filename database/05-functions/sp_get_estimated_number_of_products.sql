CREATE OR REPLACE FUNCTION sp_get_estimated_number_of_products(_siteid smallint DEFAULT NULL::smallint, _sentinel_1 boolean DEFAULT FALSE::boolean)
  RETURNS integer AS
$BODY$
DECLARE total_1 integer;
DECLARE total_2 integer;
DECLARE total_3 integer DEFAULT 0::integer;
BEGIN
	SELECT COALESCE(SUM(product_count),0)  into total_1 FROM downloader_count
	WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,min(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid ) AND product_count >=0 AND satellite_id=1  GROUP BY site_id );

	SELECT COALESCE(SUM(product_count),0)  into total_2 FROM downloader_count
	WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,min(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid) AND product_count >=0 AND satellite_id=2 GROUP BY site_id );

	IF(_sentinel_1) THEN
		SELECT COALESCE(SUM(product_count),0)  into total_3 FROM downloader_count
		WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,min(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid) AND product_count >=0 AND satellite_id=3 GROUP BY site_id );
	END IF;

	RETURN (total_1 + total_2 + total_3) as nr;
END
$BODY$
  LANGUAGE plpgsql STABLE;
