-- Function: sp_get_job_definition(integer)

-- DROP FUNCTION sp_get_job_definition(integer);

CREATE OR REPLACE FUNCTION sp_get_job_definition(IN _job_id integer)
  RETURNS TABLE(processor_id smallint, site_id smallint, parameters json) AS
$BODY$
BEGIN

RETURN QUERY SELECT job.processor_id, job.site_id, job.parameters FROM job WHERE job.id = _job_id;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION sp_get_job_definition(integer)
  OWNER TO admin;
