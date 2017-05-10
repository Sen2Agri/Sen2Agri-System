-- Function: sp_get_dashboard_products(smallint, smallint)

-- DROP FUNCTION sp_get_dashboard_products(smallint, smallint);

CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
    _site_id smallint DEFAULT NULL::smallint,
    _processor_id smallint DEFAULT NULL::smallint)
  RETURNS TABLE (
    json json
) AS
$BODY$
DECLARE q text;
BEGIN
    q := $sql$
        WITH site_names(id, name, geog, row) AS (
                select id, name, st_astext(geog), row_number() over (order by name)
                from site
            ),
            product_type_names(id, name, description, row) AS (
                select id, name, description, row_number() over (order by description)
                from product_type
            ),
            data(id, satellite_id, product,product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
            SELECT
                P.id,
                P.satellite_id,
                P.name,
                PT.name,
                PT.description,
                PR.name,
                S.name,
                P.full_path,
                P.quicklook_image,
                P.footprint,
                P.created_timestamp,
                S.geog
            FROM product P
                JOIN product_type_names PT ON P.product_type_id = PT.id
                JOIN processor PR ON P.processor_id = PR.id
                JOIN site_names S ON P.site_id = S.id
            WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
    $sql$;
    IF $1 IS NOT NULL THEN
        q := q || $sql$
                AND P.site_id = $1
                AND EXISTS (
                    SELECT * FROM season WHERE season.site_id = $1 AND P.created_timestamp BETWEEN season.start_date AND season.end_date
                )
        $sql$;
    END IF;
    IF $2 IS NOT NULL THEN
        q := q || $sql$
                AND P.product_type_id = $2
        $sql$;
    END IF;

    q := q || $sql$
            ORDER BY S.row, PT.row, P.name
        )
--         select * from data;
        SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
    $sql$;

--     raise notice '%', q;

    RETURN QUERY
    EXECUTE q
    USING _site_id, _processor_id;
END
$BODY$
  LANGUAGE plpgsql STABLE
  COST 100;
ALTER FUNCTION sp_get_dashboard_products(smallint, smallint)
  OWNER TO admin;
