CREATE OR REPLACE FUNCTION sp_get_configuration_parameters(
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (key CHARACTER VARYING, value CHARACTER VARYING, type t_data_types) AS $$
BEGIN

RETURN QUERY SELECT config.key, config.value, config.type FROM config WHERE CASE WHEN _prefix IS NOT NULL THEN config.key like _prefix || '%' ELSE 1 = 1 END;

END;
$$ LANGUAGE plpgsql;