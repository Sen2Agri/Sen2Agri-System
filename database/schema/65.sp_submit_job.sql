CREATE OR REPLACE FUNCTION sp_submit_job(
IN _name character varying,
IN _description character varying,
IN _processor_id smallint,
IN _site_id smallint,
IN _start_type_id smallint,
IN _parameters json,
IN _configuration json
) RETURNS int AS $$
DECLARE return_id int;
BEGIN

	INSERT INTO job(
	processor_id, 
	site_id, 
	start_type_id, 
	parameters, 
	submit_timestamp, 
	status_id, 
	status_timestamp)
	VALUES (
	_processor_id, 
	_site_id, 
	_start_type_id, 
	_parameters, 
	now(), 
	1, -- Submitted
	now()) RETURNING id INTO return_id;

    WITH configCTE AS (
        SELECT "key",
               "value" AS global_value
        FROM config
        WHERE config.site_id IS NULL
    ),
    configValues AS (
        SELECT "key",
               COALESCE((SELECT "value"
                        FROM config
                        WHERE config.key = configCTE.key
                        AND config.site_id = _site_id), global_value) AS "value"
        FROM configCTE
    )
    INSERT INTO config_job(
        "job_id",
        "key",
        "value")
    SELECT return_id,
           "key",
           "value"
    FROM configValues;

    PERFORM
        sp_upsert_job_parameters(return_id, _configuration);

	INSERT INTO event(
	type_id,
	data,
	submitted_timestamp)
	VALUES (
	7, -- JobSubmitted
	('{"job_id":' || return_id || ', "processor_id":' || _processor_id || ', "parameters":' || _parameters || '}') :: json,
	now()
	);

	RETURN return_id;

END;
$$ LANGUAGE plpgsql;