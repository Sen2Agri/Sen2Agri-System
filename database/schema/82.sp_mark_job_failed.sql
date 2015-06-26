CREATE OR REPLACE FUNCTION sp_mark_job_failed(
IN _job_id int
) RETURNS void AS $$
BEGIN


	UPDATE task
	SET status_id = 8, -- Error
	status_timestamp = now()
	WHERE job_id = _job_id
	AND status_id NOT IN (6, 7); -- Finished or cancelled tasks can't be failed

	UPDATE job
	SET status_id = 8, -- Error
	status_timestamp = now()
	WHERE id = _job_id;

END;
$$ LANGUAGE plpgsql;

