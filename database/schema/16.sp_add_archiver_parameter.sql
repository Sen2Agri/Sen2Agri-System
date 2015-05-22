CREATE OR REPLACE FUNCTION sp_add_archiver_parameter(
IN _processor_id SMALLINT,
IN _product_id SMALLINT,
IN _min_age SMALLINT) RETURNS INTEGER AS $$
DECLARE
    id_out integer;
BEGIN

	INSERT INTO config_archiver( 
	processor_id, 
	product_id, 
	min_age)
	VALUES( 
	_processor_id, 
	_product_id, 
	_min_age) returning id into id_out;

	RETURN id_out;

END;
$$ LANGUAGE plpgsql;