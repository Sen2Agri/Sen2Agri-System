CREATE OR REPLACE FUNCTION sp_pad_right_json_history_array(
IN _history json,
IN _since timestamp,
IN _interval varchar
) 
RETURNS json AS $$
DECLARE temp_array json[];
DECLARE temp_json json;
DECLARE previous_timestamp timestamp;
DECLARE to_timestamp timestamp;
BEGIN

	-- Get the array of timestamp - value json pairs
	SELECT array_agg(history_array.value::json) INTO temp_array FROM (SELECT * FROM json_array_elements(_history)) AS history_array;

	-- The previous timestamp always starts from now
	previous_timestamp := now();

	-- If the array is not empty, get the newest timestamp; otherwise use _since as the oldest entry to go to
	IF temp_array IS NULL OR array_length(temp_array,1) = 0 THEN
		to_timestamp := _since;
	ELSE
		to_timestamp := TIMESTAMP 'epoch' + (temp_array[array_length(temp_array, 1)]::json->>0)::bigint / 1000 * INTERVAL '1 second';
	END IF;

	-- Add values to the right of the array until the desired "to" timestamp is reached
	LOOP
		-- Compute the new previous timestamp
		previous_timestamp := previous_timestamp - _interval::interval;

		-- If using the new previous timestamp would take the array beyond the to, or beyond the _since, break. This keeps the array from growing larger than needed.
		IF previous_timestamp < to_timestamp OR previous_timestamp < _since THEN
			EXIT;  
		END IF;
		
		temp_json := json_build_array(extract(epoch from previous_timestamp)::bigint * 1000, null);
		temp_array := array_append(temp_array, temp_json);
	END LOOP;

	temp_json := array_to_json(temp_array);

	RETURN temp_json;
	
END;
$$ LANGUAGE plpgsql;