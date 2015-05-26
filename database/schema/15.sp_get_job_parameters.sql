CREATE OR REPLACE FUNCTION sp_get_job_parameters(
IN _id_job INT,
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (key CHARACTER VARYING, value CHARACTER VARYING) AS $$
BEGIN

RETURN QUERY SELECT config_job.key, config_job.value FROM config_job WHERE config_job.id_job =  _id_job AND CASE WHEN _prefix IS NOT NULL THEN config_job.key like 'prefix%' ELSE 1 = 1 END;

END;
$$ LANGUAGE plpgsql;