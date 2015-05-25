CREATE OR REPLACE FUNCTION sp_get_parameter_set(
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (key CHARACTER VARYING, friendly_name character varying, value CHARACTER VARYING, type t_data_type, is_advanced boolean, config_category_id smallint, site_id smallint, last_updated timestamp with time zone) AS $$
BEGIN

RETURN QUERY SELECT config.key, config.friendly_name, config.value, config.type, config.is_advanced, config.config_category_id, config.site_id, config.last_updated 
FROM config WHERE CASE WHEN _prefix IS NOT NULL THEN config.key like 'prefix%' ELSE 1 = 1 END;

END;
$$ LANGUAGE plpgsql;