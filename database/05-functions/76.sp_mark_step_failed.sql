CREATE OR REPLACE FUNCTION sp_mark_step_failed(
IN _task_id int,
IN _step_name character varying,
IN _node character varying,
IN _exit_code int,
IN _user_cpu_ms bigint,
IN _system_cpu_ms bigint,
IN _duration_ms bigint,
IN _max_rss_kb int,
IN _max_vm_size_kb int,
IN _disk_read_b bigint,
IN _disk_write_b bigint,
IN _stdout_text CHARACTER VARYING,
IN _stderr_text CHARACTER VARYING
) RETURNS void AS $$
DECLARE job_id int;
BEGIN

	IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
		RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
	END IF;

	UPDATE step
	SET status_id = CASE status_id
                        WHEN 1 THEN 8 -- Submitted -> Error
                        WHEN 2 THEN 8 -- PendingStart -> Error
                        WHEN 4 THEN 8 -- Running -> Error
                        ELSE status_id
                    END,
	end_timestamp = now(),
	status_timestamp = CASE status_id
                           WHEN 1 THEN now()
                           WHEN 2 THEN now()
                           WHEN 4 THEN now()
                           ELSE status_timestamp
                       END,
	exit_code = _exit_code
	WHERE name = _step_name AND task_id = _task_id 
	AND status_id != 8; -- Prevent resetting the status on serialization error retries.

	UPDATE task
	SET status_id = CASE status_id
                        WHEN 1 THEN 8 -- Submitted -> Error
                        WHEN 4 THEN 8 -- Running -> Error
                        ELSE status_id
                    END,
	status_timestamp = CASE status_id
                           WHEN 1 THEN now()
                           WHEN 4 THEN now()
                           ELSE status_timestamp
                       END
	WHERE id = _task_id
	AND status_id != 8; -- Prevent resetting the status on serialization error retries.

	-- Make sure the statistics are inserted only once.
	IF NOT EXISTS (SELECT * FROM step_resource_log WHERE step_name = _step_name AND task_id = _task_id) THEN
		INSERT INTO step_resource_log(
		step_name, 
		task_id, 
		node_name, 
		entry_timestamp, 
		duration_ms, 
		user_cpu_ms, 
		system_cpu_ms, 
		max_rss_kb, 
		max_vm_size_kb, 
		disk_read_b, 
		disk_write_b,
		stdout_text,
		stderr_text)
		VALUES (
		_step_name, 
		_task_id, 
		_node_name, 
		now(), 
		_duration_ms, 
		_user_cpu_ms, 
		_system_cpu_ms, 
		_max_rss_kb, 
		_max_vm_size_kb, 
		_disk_read_b, 
		_disk_write_b,
		_stdout_text,
		_stderr_text);
	END IF;
	
	IF EXISTS (SELECT * FROM step WHERE task_id = _task_id AND name = _step_name AND status_id = 8) THEN
		
		SELECT task.job_id INTO job_id FROM task WHERE task.id = _task_id;
	
		INSERT INTO event(
		type_id, 
		data, 
		submitted_timestamp)
		VALUES (
		8, -- StepFailed
		('{"job_id":' || job_id || ',"task_id":' || _task_id || ',"step_name":' || _step_name || '}') :: json,
		now()
		);
	END IF;

END;
$$ LANGUAGE plpgsql;

