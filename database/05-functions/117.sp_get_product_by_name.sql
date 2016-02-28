CREATE OR REPLACE FUNCTION sp_get_product_by_name(
    IN _name character varying)
  RETURNS TABLE(product_type_id smallint, processor_id smallint, site_id smallint, full_path character varying) AS
$BODY$
BEGIN

RETURN QUERY SELECT product.product_type_id, product.processor_id, product.site_id, product.full_path
FROM product
WHERE name = _name;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION sp_get_product_by_name(character varying)
  OWNER TO admin;
