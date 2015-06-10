CREATE OR REPLACE FUNCTION sp_build_archive_path(
IN _archive_path VARCHAR,
IN _site VARCHAR,
IN _processor VARCHAR,
IN _product VARCHAR,
IN _current_path VARCHAR) RETURNS 
varchar AS $$
DECLARE return_value VARCHAR;
BEGIN
	
	return_value := regexp_replace(_archive_path, '\{site\}', _site);
	return_value := regexp_replace(return_value, '\{processor\}', _processor);
	return_value := regexp_replace(return_value, '\{product\}', _product);

	return_value := return_value || '/' || regexp_replace(_current_path, '^.+[/\\]+([^/\\]+)[/\\]+[^/\\]*$', '\1') || '/';
	
	RETURN regexp_replace(return_value, '[/\\]{2,}', '/');

END;
$$ LANGUAGE plpgsql;