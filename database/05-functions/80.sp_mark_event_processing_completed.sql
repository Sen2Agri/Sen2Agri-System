CREATE OR REPLACE FUNCTION sp_mark_event_processing_completed(
IN _event_id int
) RETURNS void AS $$
BEGIN

	UPDATE event
	SET processing_completed_timestamp = now()
	WHERE id = _event_id;

END;
$$ LANGUAGE plpgsql;

