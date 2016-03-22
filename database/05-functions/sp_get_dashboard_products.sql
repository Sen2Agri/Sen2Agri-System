-- Function: sp_get_dashboard_products(smallint, smallint)

-- DROP FUNCTION sp_get_dashboard_products(smallint, smallint);

CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
    site_id smallint DEFAULT NULL::smallint,
    processor_id smallint DEFAULT NULL::smallint)
  RETURNS json AS
$BODY$
DECLARE return_string text;
BEGIN
	WITH data(product,product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp,site_coord) AS (
		SELECT 	P.name, 
			PT.name,
            PT.description,            
			PR.name,
			S.name,
			P.full_path,
			P.quicklook_image,
			P.footprint,
			P.created_timestamp,
			st_astext(S.geog)
  		FROM product P
			JOIN product_type PT ON P.product_type_id = PT.id
			JOIN processor PR ON P.processor_id = PR.id
			JOIN site S ON P.site_id = S.id
		WHERE
			($1 IS NULL OR P.site_id = $1)
			AND ($2 IS NULL OR P.processor_id = $2)
			AND COALESCE(P.is_archived, FALSE) = FALSE
        ORDER BY S.name, PT.description, P.name
	)

	SELECT array_to_json(array_agg(row_to_json(data)),true) INTO return_string FROM data;
	RETURN return_string::json;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION sp_get_dashboard_products(smallint, smallint)
  OWNER TO admin;
