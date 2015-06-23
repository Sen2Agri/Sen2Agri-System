CREATE OR REPLACE FUNCTION sp_upsert_parameters(
IN _parameters JSON,
IN _is_admin BOOLEAN) RETURNS
TABLE (key CHARACTER VARYING, error_message CHARACTER VARYING) AS $$
BEGIN

	CREATE TEMP TABLE params (
		key character varying,
		site_id smallint,
		friendly_name character varying,
		value character varying,
		type t_data_type,
		is_advanced boolean,
		config_category_id smallint,
		is_valid_error_message character varying) ON COMMIT DROP;

	-- Parse the JSON and fill the temporary table.
	BEGIN
		INSERT INTO params
		SELECT * FROM json_populate_recordset(null::params, _parameters);
	EXCEPTION WHEN OTHERS THEN
		RAISE EXCEPTION 'JSON did not match expected format or incorrect values were found. Error: %', SQLERRM USING ERRCODE = 'UE001';
	END;

	-- Get the type from the table for those values that already exist.
	UPDATE params 
	SET type = config_metadata.type
	FROM config_metadata
	WHERE params.key = config_metadata.key AND params.type IS NULL;

	-- Validate the values against the expected data type.
	UPDATE params
	SET is_valid_error_message = sp_validate_data_type_value(value, type);

        IF NOT _is_admin THEN
            -- Make sure not to update advanced parameters if the caller is not an admin
            UPDATE params
            SET is_valid_error_message = 'Only an administrator can update this parameter'
            FROM config_metadata
            WHERE params.key = config_metadata.key and config_metadata.is_advanced;
        END IF;

	-- Update the values for the params that do exist, that are not to be deleted and that are valid
	UPDATE config SET
		key = params.key,
		value = params.value,
		last_updated = now()
	FROM  params
        WHERE config.key = params.key AND config.site_id IS NOT DISTINCT FROM params.site_id AND params.value IS NOT NULL AND params.is_valid_error_message IS NULL;

	-- Update the metadata for the params that do exist, that are not to be deleted and that are valid
        UPDATE config_metadata SET
		friendly_name = coalesce(params.friendly_name, config_metadata.friendly_name),
		type = coalesce(params.type, config_metadata.type),
		is_advanced = coalesce(params.is_advanced, config_metadata.is_advanced),
		config_category_id = coalesce(params.config_category_id, config_metadata.config_category_id)
	FROM  params
        WHERE config_metadata.key = params.key AND params.value IS NOT NULL AND params.is_valid_error_message IS NULL;

	-- Insert the values for the params that do not exist, that are not to be deleted and that are valid
	INSERT INTO config(
		key,
		site_id,
		value)
        SELECT 
		params.key,
		params.site_id,
		params.value		
	FROM params
	WHERE params.key IS NOT NULL AND params.value IS NOT NULL AND params.is_valid_error_message IS NULL AND
	NOT EXISTS (SELECT * FROM config WHERE config.key = params.key AND config.site_id IS NOT DISTINCT FROM params.site_id);

	-- Insert the metadat for the params that do not exist, that are not to be deleted and that are valid
	INSERT INTO config_metadata(
		key,
		friendly_name,
		type,
		is_advanced,
		config_category_id)
        SELECT 
		params.key,
		coalesce(params.friendly_name, ''),
		coalesce(params.type, 'string'),
		coalesce(params.is_advanced, false),
		coalesce(params.config_category_id, 1)
	FROM params
	WHERE params.key IS NOT NULL AND params.value IS NOT NULL AND params.is_valid_error_message IS NULL AND
	NOT EXISTS (SELECT * FROM config_metadata WHERE config_metadata.key = params.key);

	-- Delete the values that receive NULL as value since this is the marked for the delete action
	DELETE FROM config
	WHERE id IN (SELECT config.id FROM config INNER JOIN params ON config.key = params.key AND config.site_id IS NOT DISTINCT FROM params.site_id AND params.value IS NULL);

	-- Report any possible errors
	RETURN QUERY SELECT params.key as key, params.is_valid_error_message as error_message FROM params WHERE params.is_valid_error_message IS NOT NULL;
	
END;
$$ LANGUAGE plpgsql;