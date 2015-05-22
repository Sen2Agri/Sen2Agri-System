CREATE OR REPLACE FUNCTION sp_modify_archiver_parameter(
IN _id INT,
IN _processor_id SMALLINT,
IN _product_id SMALLINT,
IN _min_age SMALLINT) RETURNS VOID AS $$
BEGIN

	UPDATE config_archiver SET 
	processor_id = _processor_id,
	product_id = _product_id, 
	min_age = _min_age
	WHERE id = _id;

END;
$$ LANGUAGE plpgsql;