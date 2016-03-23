CREATE OR REPLACE FUNCTION sp_get_dashboard_landsat_tiles(site_id smallint)
  RETURNS json AS
$BODY$
DECLARE return_string text;
BEGIN
	WITH data(code,geometry) AS (
	     SELECT  DISTINCT 
	     	     WRSD.pr, 
		     ST_AsGeoJSON(WRSD.geog)
	     FROM shape_tiles_l8 WRSD
  	     JOIN site S ON ST_Intersects(S.geog, WRSD.geog)
	     WHERE
		     S.id = $1
        )

	SELECT array_to_json(array_agg(row_to_json(data)),true) INTO return_string FROM data;
	RETURN return_string::json;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
