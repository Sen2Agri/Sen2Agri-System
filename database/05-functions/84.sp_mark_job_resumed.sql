CREATE OR REPLACE FUNCTION sp_mark_job_resumed(
IN _job_id int
) RETURNS void AS $$
DECLARE unrunnable_task_ids int[];
DECLARE runnable_task_ids int[];
DECLARE runnable_task_id int;
DECLARE processor_id processor.id%TYPE;
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

	-- Get the list of tasks that depended on tasks that have NOT been finished
	SELECT array_agg(task.id) INTO unrunnable_task_ids FROM task
	WHERE task.job_id = _job_id AND	EXISTS (SELECT * FROM task AS task2 WHERE task2.id = ANY (task.preceding_task_ids) AND task2.status_id != 6 /*Finished*/ );

	-- Get the list of tasks that depended on tasks that HAVE been finished
	SELECT array_agg(task.id) INTO runnable_task_ids FROM task
	WHERE task.job_id = _job_id AND NOT EXISTS (SELECT * FROM task AS task2 WHERE task2.id = ANY (task.preceding_task_ids) AND task2.status_id != 6 /*Finished*/ );

	-- Update the tasks that CANNOT be started right now
	UPDATE task
	SET status_id = 3, --NeedsInput
	status_timestamp = now()
	WHERE id = ANY (unrunnable_task_ids)
	AND status_id = 5; --Paused

	-- Update the tasks that CAN be started right now
	UPDATE task
	SET status_id = CASE WHEN EXISTS (SELECT * FROM step WHERE step.task_id = task.id AND step.status_id = 6) THEN 4 --Running
			ELSE 1 --Submitted
			END,
	status_timestamp = now()
	WHERE id = ANY (runnable_task_ids)
	AND status_id = 5; --Paused

    processor_id := (SELECT job.processor_id FROM job WHERE id = _job_id);

    IF runnable_task_ids IS NOT NULL THEN
        -- Add events for all the runnable tasks
        FOREACH runnable_task_id IN ARRAY runnable_task_ids
        LOOP
                INSERT INTO event(
                type_id,
                data,
                submitted_timestamp)
                VALUES (
                1, -- TaskRunnable
                ('{"job_id":' || _job_id || ', "processor_id":' || processor_id || ', "task_id":' || runnable_task_id || '}') :: json,
                now());
        END LOOP;
    END IF;

	UPDATE job
	SET status_id = CASE WHEN EXISTS (SELECT * FROM task WHERE task.job_id = job.id AND status_id IN (4,6)) THEN 4 --Running
			ELSE 1 --Submitted
			END,
	status_timestamp = now()
	WHERE id = _job_id
	AND status_id = 5; --Paused

END;
$$ LANGUAGE plpgsql;

