CREATE OR REPLACE FUNCTION sp_upsert_subscription_parameters(
IN _job_id INT,
IN _parameters JSON) RETURNS 
TABLE (key CHARACTER VARYING, error_message CHARACTER VARYING) AS $$
BEGIN

	CREATE TEMP TABLE params (
		key character varying,
		value character varying,
		type t_data_type,
		is_valid_error_message character varying) ON COMMIT DROP;

	BEGIN
		INSERT INTO params
		SELECT * FROM json_populate_recordset(null::params, _parameters);
	EXCEPTION WHEN OTHERS THEN
		RAISE EXCEPTION 'JSON did not match expected format or incorrect values were found. Error: %', SQLERRM USING ERRCODE = 'UE001';
	END;

	-- Get the parameter types from the main table.
	UPDATE params 
	SET type = config.type
	FROM config
	WHERE params.key = config.key;

	-- Validate the values against the expected data types.
	UPDATE params
	SET is_valid_error_message = sp_validate_data_type_value(value, type);

	-- Use a proper message for the params not found in the main table
	UPDATE params
	SET is_valid_error_message = 'Parameter not found in config table.'
	WHERE NOT EXISTS (SELECT * FROM config WHERE params.key = config.key);

	-- Update the ones that do exist in the current table and are valid
	UPDATE config_subscription SET
		value = params.value
	FROM  params
        WHERE config_subscription.key = params.key AND params.is_valid_error_message IS NULL;

	-- Insert the ones that do not exist in the current table, that can be found in the main table and are also valid
	INSERT INTO config_subscription(
		key,
		value)
        SELECT 
		params.key,
		params.value
	FROM params INNER JOIN config ON params.key = config.key
	WHERE params.key IS NOT NULL AND params.is_valid_error_message IS NULL AND
	NOT EXISTS (SELECT * FROM config_subscription WHERE config_subscription.key = params.key);

	-- Report any possible errors
	RETURN QUERY SELECT params.key as key, params.is_valid_error_message as error_message FROM params WHERE params.is_valid_error_message IS NOT NULL;
	
END;
$$ LANGUAGE plpgsql;