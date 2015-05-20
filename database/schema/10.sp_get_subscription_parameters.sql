CREATE OR REPLACE FUNCTION sp_get_subscription_parameters(
IN _id_job INT,
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (key CHARACTER VARYING, value CHARACTER VARYING) AS $$
BEGIN

RETURN QUERY SELECT config_subscription.key, config_subscription.value FROM config_subscription WHERE config_subscription.id_job =  _id_job AND CASE WHEN _prefix IS NOT NULL THEN config.key like 'prefix%' ELSE 1 = 1 END;

END;
$$ LANGUAGE plpgsql;