CREATE OR REPLACE FUNCTION sp_remove_archiver_parameter(
IN _id INT) RETURNS VOID AS $$
BEGIN

	DELETE FROM config_archiver 
	WHERE id = _id;

END;
$$ LANGUAGE plpgsql;