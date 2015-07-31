CREATE OR REPLACE FUNCTION sp_mark_job_needs_input(
IN _job_id int
) RETURNS void AS $$
BEGIN

	UPDATE job
	SET status_id = 3, -- NeedsInput
	status_timestamp = now()
	WHERE id = _job_id;

END;
$$ LANGUAGE plpgsql;

