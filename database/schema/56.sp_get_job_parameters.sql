CREATE OR REPLACE FUNCTION sp_get_job_parameters(
IN _job_id INT,
IN _prefix CHARACTER VARYING DEFAULT NULL) RETURNS 
TABLE ("key" CHARACTER VARYING, "value" CHARACTER VARYING) AS $$
BEGIN

RETURN QUERY
    SELECT config_job.key,
           config_job.value
    FROM config_job
    WHERE config_job.job_id = _job_id
      AND CASE
              WHEN _prefix IS NOT NULL THEN config_job.key like _prefix || '%'
              ELSE 1 = 1
          END;

END;
$$ LANGUAGE plpgsql;

