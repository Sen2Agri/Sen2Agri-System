CREATE OR REPLACE FUNCTION sp_mark_products_archived(
IN _products JSON) RETURNS VOID AS $$
BEGIN

	CREATE TEMP TABLE products (
		id int,
		new_path varchar) ON COMMIT DROP;

	-- Parse the JSON and fill the temporary table.
	BEGIN
		INSERT INTO products
		SELECT * FROM json_populate_recordset(null::products, _products);
	EXCEPTION WHEN OTHERS THEN
		RAISE EXCEPTION 'JSON did not match expected format or incorrect values were found. Error: %', SQLERRM USING ERRCODE = 'UE001';
	END;

	-- Update the table
	UPDATE product
	SET 
	full_path = products.new_path,
	is_archived = true,
	archived = now()
	FROM products WHERE product.id = products.id;

END;
$$ LANGUAGE plpgsql;