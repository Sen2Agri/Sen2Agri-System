CREATE OR REPLACE FUNCTION sp_update_step_status(
IN _step_name character varying,
IN _task_id int,
IN _status_id smallint
) RETURNS void AS $$
BEGIN

	UPDATE step
	SET status_id = _status_id, 
	status_timestamp = now()
	WHERE task_id= _task_id AND name = _step_name;

END;
$$ LANGUAGE plpgsql;

