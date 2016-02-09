CREATE OR REPLACE FUNCTION sp_submit_task(
IN _job_id int,
IN _module_short_name character varying,
IN _parameters json,
IN _preceding_task_ids json
) RETURNS int AS $$
DECLARE return_id int;
DECLARE preceding_task_ids int[];
DECLARE status_id smallint;
DECLARE processor_id processor.id%TYPE;
BEGIN

	BEGIN
		SELECT array_agg(value) INTO  preceding_task_ids FROM json_array_elements(_preceding_task_ids);
	EXCEPTION WHEN OTHERS THEN
		RAISE EXCEPTION '_preceding_task_ids JSON did not match expected format or incorrect values were found. Error: %', SQLERRM USING ERRCODE = 'UE001';
	END;

	IF coalesce(array_length(preceding_task_ids, 1),0) = 0 THEN
		status_id := 1; --Submitted
	ELSE
		status_id := 3; --NeedsInput
	END IF;

    processor_id := (SELECT processor_id FROM job WHERE id = _job_id);

	INSERT INTO task(
	job_id,
	module_short_name,
	parameters,
	submit_timestamp,
	status_id,
	status_timestamp,
	preceding_task_ids)
	VALUES (
	_job_id,
	_module_short_name,
	_parameters,
	now(),
	status_id,
	now(),
	preceding_task_ids) RETURNING id INTO return_id;

	IF status_id = 1 THEN  -- Submitted
		INSERT INTO event(
		type_id,
		data,
		submitted_timestamp)
		VALUES (
		1, -- TaskRunnable
		('{"job_id":' || _job_id || ', "processor_id":' || processor_id || ', "task_id":' || return_id || '}') :: json,
		now()
		);
	END IF;

	RETURN return_id;

END;
$$ LANGUAGE plpgsql;
