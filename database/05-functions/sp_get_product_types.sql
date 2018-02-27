-- Function: sp_get_product_types()

-- DROP FUNCTION sp_get_product_types();

CREATE OR REPLACE FUNCTION sp_get_product_types()
  RETURNS TABLE(id smallint, description character varying, "name" character varying) AS
$BODY$
BEGIN
    RETURN QUERY
        SELECT product_type.id,
               product_type.description,
               product_type.name
        FROM product_type
        ORDER BY product_type.id;
END
$BODY$
  LANGUAGE plpgsql STABLE