CREATE OR REPLACE FUNCTION sp_get_archiver_parameters() RETURNS 
TABLE (id INT, processor_id smallint, product_id smallint, min_age smallint) AS $$
BEGIN

RETURN QUERY SELECT config_archiver.id, config_archiver.processor_id, config_archiver.product_id, config_archiver.min_age FROM config_archiver;

END;
$$ LANGUAGE plpgsql;