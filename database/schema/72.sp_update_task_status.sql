CREATE OR REPLACE FUNCTION sp_update_task_status(
IN _task_id int,
IN _status_id smallint
) RETURNS void AS $$
BEGIN

	UPDATE task
	SET status_id = _status_id, 
	status_timestamp = now()
	WHERE id= _task_id;

END;
$$ LANGUAGE plpgsql;

