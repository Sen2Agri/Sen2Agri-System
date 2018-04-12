
-- Function: sp_get_products_sites(integer[])

CREATE OR REPLACE FUNCTION sp_get_products_sites(IN _site_id integer[] DEFAULT NULL::integer[])
RETURNS TABLE(id smallint, "name" character varying, short_name character varying, enabled boolean) AS
$BODY$
        BEGIN
        RETURN QUERY
                SELECT DISTINCT (S.id),
                    S.name,
                    S.short_name,
                    S.enabled
                FROM site S
                JOIN product P ON P.site_id = S.id
                WHERE _site_id IS NULL OR S.id = ANY(_site_id)
                ORDER BY S.name;
        END
$BODY$
LANGUAGE plpgsql VOLATILE