CREATE OR REPLACE FUNCTION sp_dashboard_add_site(
    _name character varying,
    _geog character varying,
    _enabled boolean)
  RETURNS smallint AS
$BODY$
DECLARE _short_name character varying;
DECLARE return_id smallint;
BEGIN

    _short_name := lower(_name);
    _short_name := regexp_replace(_short_name, '\W+', '_', 'g');
    _short_name := regexp_replace(_short_name, '_+', '_', 'g');
    _short_name := regexp_replace(_short_name, '^_', '');
    _short_name := regexp_replace(_short_name, '_$', '');

	INSERT INTO site (name, short_name, geog, enabled)
		VALUES (_name, _short_name, ST_Multi(ST_Force2D(ST_GeometryFromText(_geog))) :: geography, _enabled)
	RETURNING id INTO return_id;

	RETURN return_id;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
