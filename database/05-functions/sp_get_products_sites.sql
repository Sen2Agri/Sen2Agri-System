
-- Function: sp_get_products_sites(smallint)

-- DROP FUNCTION sp_get_products_sites(smallint);

CREATE OR REPLACE FUNCTION sp_get_products_sites(IN _site_id smallint DEFAULT NULL::smallint)
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
                        WHERE _site_id IS NULL OR S.id = _site_id
                        ORDER BY S.name;
                END
                $BODY$
  LANGUAGE plpgsql VOLATILE