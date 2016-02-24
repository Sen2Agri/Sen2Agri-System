CREATE OR REPLACE FUNCTION sp_insert_event(
IN _type_id int,
IN _data json 
) RETURNS int AS $$
DECLARE _return_id int;
BEGIN

	INSERT INTO event(
	type_id, 
	data)
	VALUES (
	_type_id,
	_data
	) RETURNING id INTO _return_id;

	RETURN _return_id;

END;
$$ LANGUAGE plpgsql;

