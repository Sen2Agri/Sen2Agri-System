CREATE OR REPLACE FUNCTION sp_get_parameters(
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (key CHARACTER VARYING, site_id smallint, value CHARACTER VARYING) AS $$
BEGIN

RETURN QUERY SELECT config.key, config.site_id, config.value 
FROM config 
WHERE CASE WHEN _prefix IS NOT NULL THEN config.key like _prefix || '%' ELSE 1 = 1 END
ORDER BY config.key;

END;
$$ LANGUAGE plpgsql;