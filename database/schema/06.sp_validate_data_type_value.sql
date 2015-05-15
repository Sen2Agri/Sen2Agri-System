CREATE OR REPLACE FUNCTION sp_validate_data_type_value(
IN _value VARCHAR,
IN _type t_data_type) RETURNS VARCHAR AS $$
DECLARE boolean_value boolean;
DECLARE date_value date;
DECLARE double_value double precision;
DECLARE bigint_value bigint;
DECLARE varchar_value character varying;
BEGIN

	CASE _type
		WHEN 'bool' THEN
			BEGIN
				boolean_value := _value::boolean;
			EXCEPTION WHEN OTHERS THEN
				RETURN 'Invalid boolean value: '||_value;
			END;
		WHEN 'date' THEN
			BEGIN
				date_value := _value::date;
			EXCEPTION WHEN OTHERS THEN
				RETURN 'Invalid date value: '||_value;
			END;
		WHEN 'float' THEN
			BEGIN
				double_value := _value::double precision;
			EXCEPTION WHEN OTHERS THEN
				RETURN 'Invalid float value: '||_value;
			END;
		WHEN 'int' THEN
			BEGIN
				bigint_value := _value::bigint;
			EXCEPTION WHEN OTHERS THEN
				RETURN 'Invalid integer value: '||_value;
			END;
		WHEN 'path','string' THEN
			RETURN NULL; -- The _value is already a string.
		ELSE
			RETURN 'Data type validation not implemented';
	END CASE;

	RETURN NULL;

END;
$$ LANGUAGE plpgsql;