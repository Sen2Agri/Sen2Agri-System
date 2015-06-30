CREATE OR REPLACE FUNCTION sp_mark_job_resumed(
IN _job_id int
) RETURNS void AS $$
BEGIN

	IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
		RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
	END IF;

	UPDATE step
	SET status_id = 1, --Submitted
	status_timestamp = now()
	FROM task
	WHERE task.id = step.task_id AND task.job_id = _job_id
	AND step.status_id = 5; --Paused

	UPDATE task
	SET status_id = CASE WHEN EXISTS (SELECT * FROM step WHERE step.task_id = task.id AND step.status_id = 6) THEN 4 --Running
			ELSE 1 --Submitted
			END,
	status_timestamp = now()
	WHERE job_id = _job_id
	AND step.status_id = 5; --Paused

	UPDATE job
	SET status_id = CASE WHEN EXISTS (SELECT * FROM task WHERE task.job_id = job.id AND step.status_id IN (4,6)) THEN 4 --Running
			ELSE 1 --Submitted
			END,
	status_timestamp = now()
	WHERE id = _job_id
	AND step.status_id = 5; --Paused

END;
$$ LANGUAGE plpgsql;

