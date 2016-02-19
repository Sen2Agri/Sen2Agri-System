-- Function: sp_insert_processor(character varying, character varying, character varying)

-- DROP FUNCTION sp_insert_processor(character varying, character varying, character varying);

CREATE OR REPLACE FUNCTION sp_insert_processor(
    _name character varying,
    _description character varying,
    _short_name character varying)
  RETURNS integer AS
$BODY$
DECLARE _return_id int;
BEGIN

	INSERT INTO processor(
	name,
	description,
	short_name)
	VALUES (
	_name,
	_description,
	_short_name
	) RETURNING id INTO _return_id;

	RETURN _return_id;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION sp_insert_processor(character varying, character varying, character varying)
  OWNER TO admin;
