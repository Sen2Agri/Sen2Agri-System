CREATE OR REPLACE FUNCTION sp_get_dashboard_sites(site_id smallint DEFAULT NULL::smallint)
  RETURNS json AS
$BODY$
DECLARE return_string text;
BEGIN
	WITH data(id,name,short_name) AS (
		SELECT 	S.id, 
			S.name, 
			S.short_name
  		FROM site S
		WHERE
			($1 IS NULL OR S.id = $1)
		ORDER BY S.name
	)

	SELECT array_to_json(array_agg(row_to_json(data)),true) INTO return_string FROM data;
	RETURN return_string::json;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
