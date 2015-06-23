CREATE OR REPLACE FUNCTION sp_mark_step_started(
IN _task_id int,
IN _step_name character varying
) RETURNS void AS $$
DECLARE return_id int;
BEGIN

	IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
		RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
	END IF;

	UPDATE step
	SET status_id = 3, --Running
	status_timestamp = now()
	WHERE name = _step_name AND task_id = _task_id 
	AND status_id != 3; -- Prevent resetting the status on serialization error retries.

	UPDATE task
	SET status_id = 3, --Running
	status_timestamp = now()
	WHERE id = _task_id
	AND status_id != 3; -- Prevent resetting the status on serialization error retries.

	UPDATE job
	SET status_id = 3, --Running
	status_timestamp = now()
	FROM task WHERE job.id = task.job_id
	AND job.status_id != 3; -- Prevent resetting the status on serialization error retries.

END;
$$ LANGUAGE plpgsql;
