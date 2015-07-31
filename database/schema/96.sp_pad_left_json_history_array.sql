CREATE OR REPLACE FUNCTION sp_pad_left_json_history_array(
IN _history json,
IN _since TIMESTAMP,
IN _interval varchar
) 
RETURNS json AS $$
DECLARE temp_array json[];
DECLARE temp_json json;
DECLARE previous_timestamp timestamp;
BEGIN

	-- Get the array of timestamp - value json pairs
	SELECT array_agg(history_array.value::json) INTO temp_array FROM (SELECT * FROM json_array_elements(_history)) AS history_array;

	-- If the array is not empty, get the oldes timestamp
	IF temp_array IS NULL OR array_length(temp_array,1) = 0 THEN
		previous_timestamp := now();
	ELSE
		previous_timestamp := TIMESTAMP 'epoch' + (temp_array[1]::json->>0)::bigint / 1000 * INTERVAL '1 second';
	END IF;

	-- Add values to the left of the array until the desired "since" timestamp is reached
	LOOP
		-- Compute the new previous timestamp
		previous_timestamp := previous_timestamp - _interval::interval;

		-- If using the new previous timestamp would take the array beyond the since, break
		IF previous_timestamp < _since THEN
			EXIT;  
		END IF;
		
		temp_json := json_build_array(extract(epoch from previous_timestamp)::bigint * 1000, null);
		temp_array := array_prepend(temp_json, temp_array);
	END LOOP;

	temp_json := array_to_json(temp_array);

	RETURN temp_json;
	
END;
$$ LANGUAGE plpgsql;