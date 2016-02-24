CREATE OR REPLACE FUNCTION sp_mark_job_failed(
IN _job_id int
) RETURNS void AS $$
BEGIN
	-- Remaining tasks should be cancelled; the task that has failed has already been marked as failed.
	UPDATE task
	SET status_id = 7, -- Cancelled
	status_timestamp = now()
	WHERE job_id = _job_id
	AND status_id NOT IN (6, 7, 8); -- Finished, cancelled or failed tasks can't be cancelled

	UPDATE job
	SET status_id = 8, -- Error
	status_timestamp = now()
	WHERE id = _job_id;

END;
$$ LANGUAGE plpgsql;

