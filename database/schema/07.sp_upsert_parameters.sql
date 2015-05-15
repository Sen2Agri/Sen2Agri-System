CREATE OR REPLACE FUNCTION sp_upsert_parameters(
IN _parameters JSON) RETURNS 
TABLE (key CHARACTER VARYING, error_message CHARACTER VARYING) AS $$
BEGIN

	CREATE TEMP TABLE params (
		key character varying,
		friendly_name character varying,
		value character varying,
		type t_data_type,
		is_advanced boolean,
		config_category_id int,
		is_valid_error_message character varying) ON COMMIT DROP;

	/*CREATE TEMP TABLE return_table (
		key CHARACTER VARYING, 
		error_message CHARACTER VARYING) ON COMMIT DROP;*/

	BEGIN
		INSERT INTO params
		SELECT * FROM json_populate_recordset(null::params, _parameters);
	EXCEPTION WHEN OTHERS THEN
		RAISE EXCEPTION 'JSON did not match expected format or incorrect values were found. Error: %', SQLERRM USING ERRCODE = 'UE001';
	END;

	UPDATE params 
	SET type = config.type
	FROM config
	WHERE params.key = config.key AND params.type IS NULL;
	
	UPDATE params
	SET is_valid_error_message = sp_validate_data_type_value(value, type);

	-- Update the ones that do exist and are valid
	UPDATE config SET
		friendly_name = coalesce(params.friendly_name, config.friendly_name),
		value = params.value,
		type = coalesce(params.type, config.type),
		is_advanced = coalesce(params.is_advanced, config.is_advanced),
		config_category_id = coalesce(params.config_category_id, config.config_category_id)
	FROM  params
        WHERE config.key = params.key AND params.is_valid_error_message IS NULL;

	-- Insert the ones that do not exist and are valid
        INSERT INTO config(
		key,
		friendly_name,
		value,
		type,
		is_advanced,
		config_category_id)
        SELECT 
		params.key,
		coalesce(params.friendly_name, ''),
		params.value,
		coalesce(params.type, 'string'),
		coalesce(params.is_advanced, false),
		coalesce(params.config_category_id, 1)
        FROM params
        WHERE params.key IS NOT NULL AND params.is_valid_error_message IS NULL AND
        NOT EXISTS (SELECT * FROM config WHERE config.key = params.key);

        -- Report any possible errors
	RETURN QUERY SELECT params.key as key, params.is_valid_error_message as error_message FROM params WHERE params.is_valid_error_message IS NOT NULL;
	
END;
$$ LANGUAGE plpgsql;