-- Function: sp_get_products(smallint, smallint, timestamp with time, timestamp with time)

-- DROP FUNCTION sp_get_products(smallint, smallint, timestamp with time, timestamp with time);

CREATE OR REPLACE FUNCTION sp_get_products(
    IN site_id smallint DEFAULT NULL::smallint,
    IN product_type_id smallint DEFAULT NULL::smallint,
    IN start_time timestamp with time zone DEFAULT NULL::timestamp with time zone,
    IN end_time timestamp with time zone DEFAULT NULL::timestamp with time zone)
  RETURNS TABLE("ProductId" integer, "Product" character varying, "ProductType" character varying, "ProductTypeId" smallint, "Processor" character varying,
		"ProcessorId" smallint, "Site" character varying, "SiteId" smallint, task_id integer, full_path character varying,
		quicklook_image character varying, footprint polygon, created_timestamp timestamp with time zone) AS
$BODY$
BEGIN
	RETURN QUERY
	  	SELECT 	P.id AS ProductId,
			P.name AS Product,
  			PT.name AS ProductType,
  			P.product_type_id AS ProductTypeId,
            PR.name AS Processor,
	    P.processor_id AS ProcessorId,
            S.name AS Site,
            P.site_id AS SiteId,
            P.task_id,
            P.full_path,
            P.quicklook_image,
            P.footprint,
            P.created_timestamp
  		FROM product P
    		JOIN product_type PT ON P.product_type_id = PT.id
		    JOIN processor PR ON P.processor_id = PR.id
    		JOIN site S ON P.site_id = S.id
	    WHERE
    		($1 IS NULL OR $1 = -1 OR P.site_id = $1)
        	AND ($2 IS NULL OR $2 = -1 OR P.product_type_id = $2)
        	AND ($3 IS NULL OR P.created_timestamp >= $3)
        	AND ($4 IS NULL OR P.created_timestamp <= $4)
	        AND COALESCE(P.is_archived, FALSE) = FALSE
    	ORDER BY S.name, PT.name, P.name;
END
$BODY$
  LANGUAGE plpgsql VOLATILE;
