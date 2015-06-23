CREATE OR REPLACE FUNCTION sp_update_job_status(
IN _job_id int,
IN _status_id smallint
) RETURNS void AS $$
BEGIN

	UPDATE job
	SET status_id = _status_id, 
	status_timestamp = now()
	WHERE id= _job_id;

END;
$$ LANGUAGE plpgsql;

