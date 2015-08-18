CREATE OR REPLACE FUNCTION sp_get_dashboard_job_config(
IN _job_id INT) 
RETURNS json AS $$
DECLARE current_processor RECORD;
DECLARE temp_json json;
DECLARE temp_json2 json;
DECLARE return_string text;
BEGIN

	WITH config_params AS (
	SELECT json_build_array(key,value) AS param
	FROM config_job
	WHERE job_id = _job_id)
	SELECT array_to_json(array_agg(config_params.param)) INTO temp_json
	FROM config_params;

	WITH job_input AS (SELECT json_each(parameters) AS tuple FROM job WHERE id = _job_id)
	SELECT array_to_json(array_agg(json_build_array((job_input.tuple).key, (job_input.tuple).value))) INTO temp_json2 FROM job_input;

	return_string := json_build_object('configuration', temp_json, 'input', temp_json2);

	RETURN return_string::json;

END;
$$ LANGUAGE plpgsql;