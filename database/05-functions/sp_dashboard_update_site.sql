CREATE OR REPLACE FUNCTION sp_dashboard_update_site(
    _id smallint,
    _short_name character varying,
    _geog character varying,
    _enabled boolean)
  RETURNS void AS
$BODY$
BEGIN

IF NULLIF(_short_name, '') IS NOT NULL THEN
    UPDATE site
    SET short_name = _short_name
    WHERE id = _id;
END IF;

IF _enabled IS NOT NULL THEN
    UPDATE site
    SET enabled = _enabled
    WHERE id = _id;
END IF;

IF NULLIF(_geog, '') IS NOT NULL THEN
    UPDATE site
    SET geog = ST_Multi(ST_Force2D(ST_GeometryFromText(_geog)))
    WHERE id = _id;
END IF;

END;
$BODY$
  LANGUAGE plpgsql;
