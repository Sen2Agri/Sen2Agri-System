CREATE OR REPLACE FUNCTION sp_get_products_sites(IN _site_ids integer[] DEFAULT NULL::integer[])
RETURNS TABLE(id smallint, "name" character varying, short_name character varying, enabled boolean) AS
$BODY$
    BEGIN
    RETURN QUERY
        SELECT S.id,
               S.name,
               S.short_name,
               S.enabled
        FROM site S
        WHERE _site_ids IS NULL
           OR S.id = ANY(_site_ids)
          AND EXISTS (SELECT *
                      FROM product P
                      WHERE P.site_id = S.id)
        ORDER BY S.name;
    END
$BODY$
LANGUAGE plpgsql STABLE;
