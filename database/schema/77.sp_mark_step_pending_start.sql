CREATE OR REPLACE FUNCTION sp_mark_step_pending_start(
IN _step_name character varying,
IN _task_id int
) RETURNS void AS $$
BEGIN

	UPDATE step
	SET status_id = 2, -- PendingStart
	status_timestamp = now()
	WHERE task_id= _task_id AND name = _step_name;

END;
$$ LANGUAGE plpgsql;

