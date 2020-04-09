CREATE OR REPLACE FUNCTION sp_mark_job_finished(
IN _job_id int
) RETURNS void AS $$
BEGIN

	UPDATE job
	SET status_id = 6, --Finished
	status_timestamp = now(),
	end_timestamp = now()
	WHERE id = _job_id; 

END;
$$ LANGUAGE plpgsql;

