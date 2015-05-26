CREATE OR REPLACE FUNCTION sp_get_parameter_set(
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (key CHARACTER VARYING, site_id smallint, friendly_name character varying, value CHARACTER VARYING, type t_data_type, is_advanced boolean, config_category_id smallint, last_updated timestamp with time zone) AS $$
BEGIN

RETURN QUERY SELECT config.key, config.site_id, config_metadata.friendly_name, config.value, config_metadata.type, config_metadata.is_advanced, config_metadata.config_category_id, config.last_updated 
FROM config INNER JOIN config_metadata ON config.key = config_metadata.key
WHERE CASE WHEN _prefix IS NOT NULL THEN config.key like 'prefix%' ELSE 1 = 1 END;

END;
$$ LANGUAGE plpgsql;