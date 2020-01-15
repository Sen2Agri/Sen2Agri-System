-- Function: sp_get_l1c_products(smallint, json, json, timestamp with time zone, timestamp with time zone)

-- DROP FUNCTION sp_get_l1c_products(smallint, json, json, timestamp with time zone, timestamp with time zone);

CREATE OR REPLACE FUNCTION sp_get_l1c_products(
    IN site_id smallint DEFAULT NULL::smallint,
    IN sat_ids json DEFAULT NULL::json,
    IN status_ids json DEFAULT NULL::json,
    IN start_time timestamp with time zone DEFAULT NULL::timestamp with time zone,
    IN end_time timestamp with time zone DEFAULT NULL::timestamp with time zone)
  RETURNS TABLE("ProductId" integer, "Name" character varying, "SiteId" smallint, full_path character varying, "SatelliteId" smallint, "StatusId" smallint, product_date timestamp with time zone, created_timestamp timestamp with time zone) AS
$BODY$
DECLARE q text;
BEGIN
    q := $sql$
    WITH site_names(id, name, row) AS (
            select id, name, row_number() over (order by name)
            from site
        ),
        satellite_ids(id, name, row) AS (
            select id, satellite_name, row_number() over (order by satellite_name)
            from satellite
        )
	  	SELECT P.id AS ProductId,
			P.product_name AS Name,
            P.site_id as SiteId,
            P.full_path,
            P.satellite_id as SatelliteId,
            P.status_id as StatusId,
            P.product_date,
            P.created_timestamp
  		FROM downloader_history P
            JOIN satellite_ids SAT ON P.satellite_id = SAT.id
            JOIN site_names S ON P.site_id = S.id
    	WHERE TRUE$sql$;

    IF NULLIF($1, -1) IS NOT NULL THEN
        q := q || $sql$
            AND P.site_id = $1$sql$;
    END IF;
    IF $2 IS NOT NULL THEN
        q := q || $sql$
            AND P.satellite_id IN (SELECT value::smallint FROM json_array_elements_text($2))
        $sql$;
    END IF;
    IF $3 IS NOT NULL THEN
        q := q || $sql$
            AND P.status_id IN (SELECT value::smallint FROM json_array_elements_text($3))
        $sql$;
    END IF;
    IF $4 IS NOT NULL THEN
        q := q || $sql$
            AND P.product_date >= $4$sql$;
    END IF;
    IF $5 IS NOT NULL THEN
        q := q || $sql$
            AND P.product_date <= $5$sql$;
    END IF;
    q := q || $SQL$
        ORDER BY S.row, SAT.row, P.product_name;$SQL$;

    -- raise notice '%', q;
    
	RETURN QUERY
        EXECUTE q
        USING $1, $2, $3, $4, $5;
END
$BODY$
  LANGUAGE plpgsql STABLE;
