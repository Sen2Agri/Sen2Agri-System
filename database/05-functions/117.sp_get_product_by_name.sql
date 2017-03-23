CREATE OR REPLACE FUNCTION sp_get_product_by_name(
    _site_id site.id%TYPE,
    _name character varying)
  RETURNS TABLE(product_id smallint, product_type_id smallint, processor_id smallint, site_id smallint, full_path character varying, created_timestamp timestamp with time zone, inserted_timestamp timestamp with time zone) AS
$BODY$
BEGIN

RETURN QUERY SELECT product.product_type_id AS product_id, product.product_type_id, product.processor_id, product.site_id, product.full_path, product.created_timestamp, product.inserted_timestamp 
FROM product
WHERE product.site_id = _site_id AND
      product.name = _name;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION sp_get_product_by_name(smallint, character varying)
  OWNER TO admin;
