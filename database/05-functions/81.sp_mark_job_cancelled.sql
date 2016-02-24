CREATE OR REPLACE FUNCTION sp_mark_job_cancelled(
IN _job_id int
) RETURNS void AS $$
BEGIN

	IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
		RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
	END IF;

	UPDATE step
	SET status_id = 7, --Cancelled
	status_timestamp = now()
	FROM task
	WHERE task.id = step.task_id AND task.job_id = _job_id
	AND step.status_id NOT IN (6, 8) -- Finished or failed steps can't be cancelled
	AND step.status_id != 7; -- Prevent resetting the status on serialization error retries.

	UPDATE task
	SET status_id = 7, --Cancelled
	status_timestamp = now()
	WHERE job_id = _job_id
	AND status_id NOT IN (6, 8) -- Finished or failed tasks can't be cancelled
	AND status_id != 7; -- Prevent resetting the status on serialization error retries.

	UPDATE job
	SET status_id = 7, --Cancelled
	status_timestamp = now()
	WHERE id = _job_id
	AND status_id != 7; -- Prevent resetting the status on serialization error retries.

END;
$$ LANGUAGE plpgsql;

