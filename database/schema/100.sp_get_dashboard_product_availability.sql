CREATE OR REPLACE FUNCTION sp_get_dashboard_product_availability(
IN _since_timestamp timestamp with time zone DEFAULT null) 
RETURNS json AS $$
DECLARE current_product_type RECORD;
DECLARE temp_json json;
DECLARE temp_json2 json;
BEGIN

	IF _since_timestamp IS NULL THEN
		_since_timestamp := '1970-01-01 00:00:00'::timestamp;
	END IF;

	CREATE TEMP TABLE product_types (
		id smallint,
		name character varying,
		products json
		) ON COMMIT DROP;

	-- Get the list of processors to return the resources for
	INSERT INTO product_types (id, name)
	SELECT id, name
	FROM product_type ORDER BY name;

	-- Go through the product types and compute their data
	FOR current_product_type IN SELECT * FROM product_types ORDER BY name LOOP

		WITH products AS (
		SELECT 
			product.full_path AS "key",
			'Site: ' || site.short_name AS "info"
		FROM product
		INNER JOIN site ON product.site_id = site.id
		WHERE created_timestamp >= _since_timestamp
		AND product_type_id = current_product_type.id)
		SELECT array_to_json(array_agg(row_to_json(product_details, true))) INTO temp_json
		FROM (SELECT * FROM products) AS product_details; 

		UPDATE product_types
		SET products = json_build_object('key',current_product_type.name,'_values', temp_json)
		WHERE id = current_product_type.id;
		
	END LOOP;

	SELECT array_to_json(array_agg(product_type_details.products)) INTO temp_json
	FROM (SELECT products FROM product_types) AS product_type_details;

	temp_json2 := json_build_object('products', array_to_json(array[json_build_object('key', 'Products', '_values', temp_json)]));

	RETURN temp_json2;

END;
$$ LANGUAGE plpgsql;

