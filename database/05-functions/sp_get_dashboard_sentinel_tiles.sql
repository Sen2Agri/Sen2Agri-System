CREATE OR REPLACE FUNCTION sp_get_dashboard_sentinel_tiles(site_id smallint)
 RETURNS json
 LANGUAGE plpgsql
AS $function$
DECLARE return_string text;
BEGIN
     	WITH data(code,geometry) AS (
                SELECT
                        STS.tile_id,
                        ST_AsGeoJSON(STS.geog)
                FROM shape_tiles_s2 STS
                JOIN site S ON ST_Intersects(S.geog, STS.geog)
                WHERE
                     	S.id = $1
        )

	SELECT array_to_json(array_agg(row_to_json(data)),true) INTO return_string FROM data;
        RETURN return_string::json;
END
$function$
