CREATE OR REPLACE FUNCTION sp_mark_step_pending_start(
IN _task_id int,
IN _step_name character varying
) RETURNS void AS $$
BEGIN

	UPDATE step
	SET status_id = CASE status_id
                        WHEN 1 THEN 2 -- Submitted -> PendingStart
                        ELSE status_id
                    END,
	status_timestamp = CASE status_id
                           WHEN 1 THEN now()
                           ELSE status_timestamp
                       END
	WHERE task_id = _task_id AND name = _step_name;

END;
$$ LANGUAGE plpgsql;
