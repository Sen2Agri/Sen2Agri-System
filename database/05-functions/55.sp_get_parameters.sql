CREATE OR REPLACE FUNCTION sp_get_parameters(
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (key CHARACTER VARYING, site_id smallint, value CHARACTER VARYING) AS $$
BEGIN

RETURN QUERY SELECT config.key, config.site_id, 
			CASE WHEN _prefix IN ('downloader.summer-season.start', 'downloader.winter-season.start')
				THEN CASE WHEN CAST(left(config.value, 2) AS integer) <= CAST(config2.value AS integer)
						THEN trim(to_char(12 + CAST(left(config.value, 2) AS integer) - CAST(config2.value AS integer), '00'))||'01'
						ELSE trim(to_char(CAST(left(config.value, 2) AS integer) - CAST(config2.value AS integer), '00'))||'01' END
				ELSE config.value
			END AS value
			FROM config, config AS config2
			WHERE config2.key = 'downloader.start.offset'
				AND CASE WHEN _prefix IS NOT NULL THEN config.key like _prefix || '%' ELSE 1 = 1 END
		ORDER BY config.key;

END;
$$ LANGUAGE plpgsql;