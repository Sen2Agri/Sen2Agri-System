CREATE OR REPLACE FUNCTION sp_mark_job_paused(
IN _job_id int
) RETURNS void AS $$

BEGIN

	IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
		RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
	END IF;

	UPDATE step
	SET status_id = 5, --Paused
	status_timestamp = now()
	FROM task
	WHERE task.id = step.task_id AND task.job_id = _job_id
	AND step.status_id NOT IN (5, 6, 7, 8); -- Finished, cancelled or failed steps can't be paused

	UPDATE task
	SET status_id = 5, --Paused
	status_timestamp = now()
	WHERE job_id = _job_id
	AND status_id NOT IN (5, 6, 7, 8); -- Finished, cancelled or failed tasks can't be paused

	UPDATE job
	SET status_id = 5, --Paused
	status_timestamp = now()
	WHERE id = _job_id
	AND status_id NOT IN (5, 6, 7, 8); -- Finished, cancelled or failed jobs can't be paused

END;
$$ LANGUAGE plpgsql;

