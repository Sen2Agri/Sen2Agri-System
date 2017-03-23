-- Function: sp_get_products(smallint, smallint, timestamp with time zone, timestamp with time zone)

-- DROP FUNCTION sp_get_products(smallint, smallint, timestamp with time zone, timestamp with time zone);

CREATE OR REPLACE FUNCTION sp_get_products(
    IN site_id smallint DEFAULT NULL::smallint,
    IN product_type_id smallint DEFAULT NULL::smallint,
    IN start_time timestamp with time zone DEFAULT NULL::timestamp with time zone,
    IN end_time timestamp with time zone DEFAULT NULL::timestamp with time zone)
  RETURNS TABLE("ProductId" integer, "Product" character varying, "ProductType" character varying, "ProductTypeId" smallint, "Processor" character varying, 
                "ProcessorId" smallint, "Site" character varying, "SiteId" smallint, full_path character varying,
                quicklook_image character varying, footprint polygon, created_timestamp timestamp with time zone, inserted_timestamp timestamp with time zone) AS
$BODY$
DECLARE q text;
BEGIN
    q := $sql$
    WITH site_names(id, name, row) AS (
            select id, name, row_number() over (order by name)
            from site
        ),
        product_type_names(id, name, row) AS (
            select id, name, row_number() over (order by name)
            from product_type
        )
	  	SELECT P.id AS ProductId,
			P.name AS Product,
  			PT.name AS ProductType,
  			P.product_type_id AS ProductTypeId,
            PR.name AS Processor,
            P.processor_id AS ProcessorId,
            S.name AS Site,
            P.site_id AS SiteId,
            P.full_path,
            P.quicklook_image,
            P.footprint,
            P.created_timestamp,
            P.inserted_timestamp
  		FROM product P
            JOIN product_type_names PT ON P.product_type_id = PT.id
            JOIN processor PR ON P.processor_id = PR.id
            JOIN site_names S ON P.site_id = S.id
    	WHERE TRUE$sql$;

    IF NULLIF($1, -1) IS NOT NULL THEN
        q := q || $sql$
            AND P.site_id = $1$sql$;
    END IF;
    IF NULLIF($2, -1) IS NOT NULL THEN
        q := q || $sql$
            AND P.product_type_id = $2$sql$;
    END IF;
    IF $3 IS NOT NULL THEN
        q := q || $sql$
            AND P.created_timestamp >= $3$sql$;
    END IF;
    IF $4 IS NOT NULL THEN
        q := q || $sql$
            AND P.created_timestamp <= $4$sql$;
    END IF;
    q := q || $SQL$
        ORDER BY S.row, PT.row, P.name;$SQL$;

    -- raise notice '%', q;
    
	RETURN QUERY
        EXECUTE q
        USING $1, $2, $3, $4;
END
$BODY$
  LANGUAGE plpgsql STABLE;
