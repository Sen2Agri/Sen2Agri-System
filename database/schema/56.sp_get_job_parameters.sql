CREATE OR REPLACE FUNCTION sp_get_job_parameters(
IN _job_id INT,
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE (config_id int, key CHARACTER VARYING, site_id smallint, value CHARACTER VARYING) AS $$
BEGIN

RETURN QUERY SELECT config.id as config_id, config.key, config.site_id, config_job.value FROM config_job INNER JOIN config ON config_job.config_id = config.id
WHERE config_job.job_id = _job_id AND CASE WHEN _prefix IS NOT NULL THEN config.key like _prefix || '%' ELSE 1 = 1 END;

END;
$$ LANGUAGE plpgsql;