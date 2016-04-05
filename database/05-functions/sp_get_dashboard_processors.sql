CREATE OR REPLACE FUNCTION sp_get_dashboard_processors(processor_id smallint DEFAULT NULL::smallint)
  RETURNS json AS
$BODY$
DECLARE return_string text;
BEGIN
	WITH data(id,name,description,short_name) AS (
		SELECT 	PROC.id, 
			PROC.name,
			PROC.description, 
			PROC.short_name
  		FROM processor PROC
		WHERE
			($1 IS NULL OR PROC.id = $1)
		ORDER BY PROC.name
	)

	SELECT array_to_json(array_agg(row_to_json(data)),true) INTO return_string FROM data;
	RETURN return_string::json;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
