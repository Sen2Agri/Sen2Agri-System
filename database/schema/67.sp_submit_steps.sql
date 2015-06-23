CREATE OR REPLACE FUNCTION sp_submit_steps(
IN _steps json
) RETURNS void AS $$
BEGIN

	CREATE TEMP TABLE steps (
		name character varying,
		task_id int,
		parameters json) ON COMMIT DROP;

	-- Parse the JSON and fill the temporary table.
	BEGIN
		INSERT INTO steps
		SELECT * FROM json_populate_recordset(null::steps, _steps);
	EXCEPTION WHEN OTHERS THEN
		RAISE EXCEPTION 'JSON did not match expected format or incorrect values were found. Error: %', SQLERRM USING ERRCODE = 'UE001';
	END;

	INSERT INTO step(
	name, 
	task_id, 
	parameters, 
	submit_timestamp, 
	status_id, 
	status_timestamp)
	SELECT 
	name,
	task_id,
	parameters,
	now(), 
	1, -- Submitted
	now()
	FROM steps;

END;
$$ LANGUAGE plpgsql;