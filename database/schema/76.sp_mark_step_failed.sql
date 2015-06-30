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
IN _disk_write_b bigint
) RETURNS void AS $$
DECLARE job_id int;
BEGIN

	IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
		RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
	END IF;

	UPDATE step
	SET status_id = 8, --Error
	status_timestamp = now(), 
	exit_code = _exit_code
	WHERE name = _step_name AND task_id = _task_id 
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
		disk_write_b)
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
		_disk_write_b);
	END IF;
	
	IF EXISTS (SELECT * FROM step WHERE task_id = _task_id AND name = _step_name AND status_id = 8) 
	-- Make sure the task finished event is inserted only once.
	AND NOT EXISTS (SELECT * FROM event WHERE type_id = 7 AND data::json->'task_id' = _task_id AND data::json->'step_name' = _step_name) THEN
		
		SELECT task.job_id INTO job_id FROM task WHERE task.id = _task_id;
	
		INSERT INTO event(
		type_id, 
		data, 
		submitted_timestamp)
		VALUES (
		7, -- StepFailed
		'{"job_id":' || job_id || ',"task_id":' || _task_id || ',"step_name":' || _step_name || '}',
		now()
		);
	END IF;

END;
$$ LANGUAGE plpgsql;

