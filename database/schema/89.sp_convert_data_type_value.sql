CREATE OR REPLACE FUNCTION sp_convert_data_type_value(
IN _value VARCHAR,
IN _type t_data_type) RETURNS VARCHAR AS $$
DECLARE boolean_value boolean;
DECLARE date_value date;
DECLARE double_value double precision;
DECLARE bigint_value bigint;
DECLARE varchar_value character varying;
BEGIN

	CASE _type
		WHEN 'float' THEN
			BEGIN
				double_value := _value::double precision;
				RETURN double_value::varchar;
			EXCEPTION WHEN OTHERS THEN
				RETURN 'Invalid float value: '||_value;
			END;
		WHEN 'int' THEN
			BEGIN
				bigint_value := _value::bigint;
				RETURN bigint_value::varchar;
			EXCEPTION WHEN OTHERS THEN
				RETURN 'Invalid integer value: '||_value;
			END;
		ELSE
			RETURN 'Data type validation not implemented';
	END CASE;

	RETURN NULL;

END;
$$ LANGUAGE plpgsql;