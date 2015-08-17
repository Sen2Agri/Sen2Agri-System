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
    SET status_id = CASE status_id
                        WHEN 2 THEN 4 -- PendingStart -> Running
                        ELSE status_id
                    END,
	start_timestamp = now(),
	status_timestamp = CASE status_id
                           WHEN 2 THEN now()
                           ELSE status_timestamp
                       END
	WHERE name = _step_name AND task_id = _task_id
	AND status_id != 4; -- Prevent resetting the status on serialization error retries.

	UPDATE task
	SET status_id = CASE status_id
                        WHEN 1 THEN 4 -- Submitted -> Running
                        ELSE status_id
                    END,
	status_timestamp = CASE status_id
                           WHEN 1 THEN now()
                           ELSE status_timestamp
                       END
	WHERE id = _task_id
	AND status_id != 4; -- Prevent resetting the status on serialization error retries.

	UPDATE job
	SET status_id = CASE job.status_id
                        WHEN 1 THEN 4 -- Submitted -> Running
                        ELSE job.status_id
                    END,
	status_timestamp = CASE job.status_id
                           WHEN 1 THEN now()
                           ELSE job.status_timestamp
                       END
	FROM task WHERE job.id = task.job_id
	AND job.status_id != 4; -- Prevent resetting the status on serialization error retries.

END;
$$ LANGUAGE plpgsql;
