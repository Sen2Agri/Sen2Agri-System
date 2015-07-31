CREATE OR REPLACE FUNCTION sp_get_new_events(
) RETURNS TABLE (id int,
  type_id smallint,
  data json,
  submitted_timestamp timestamp with time zone,
  processing_started_timestamp timestamp with time zone) AS $$
BEGIN

	RETURN QUERY SELECT event.id, event.type_id, event.data, event.submitted_timestamp,
	event.processing_started_timestamp
	FROM event WHERE event.processing_completed_timestamp IS NULL;

END;
$$ LANGUAGE plpgsql;
