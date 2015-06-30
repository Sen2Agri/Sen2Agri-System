CREATE OR REPLACE FUNCTION sp_submit_task(
IN _job_id int,
IN _module_short_name character varying,
IN _parameters json
) RETURNS int AS $$
DECLARE return_id int;
BEGIN

	INSERT INTO task(
	job_id, 
	module_short_name, 
	parameters, 
	submit_timestamp, 
	status_id, 
	status_timestamp)
	VALUES (
	_job_id, 
	_module_short_name, 
	_parameters, 
	now(), 
	1, -- Submitted
	now()) RETURNING id INTO return_id;

	RETURN return_id;

END;
$$ LANGUAGE plpgsql;