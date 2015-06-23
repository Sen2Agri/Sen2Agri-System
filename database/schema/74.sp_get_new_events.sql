CREATE OR REPLACE FUNCTION sp_get_new_events(
) RETURNS TABLE (id int,
  type_id smallint,
  data json,
  submitted_timestamp timestamp with time zone,
  processing_started_timestamp timestamp with time zone) AS $$
BEGIN

	SELECT id, type_id, data, submitted_timestamp,
	processing_started_timestamp
	FROM event WHERE processing_completed_timestamp IS NULL;

END;
$$ LANGUAGE plpgsql;

