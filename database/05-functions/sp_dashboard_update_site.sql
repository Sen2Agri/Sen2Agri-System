CREATE OR REPLACE FUNCTION sp_dashboard_update_site(
    _id smallint,
    _short_name character varying,
    _geog character varying,
    _winter_season_start character varying,
    _winter_season_end character varying,
    _summer_season_start character varying,
    _summer_season_end character varying,
    _enabled boolean)
  RETURNS void AS
$BODY$
DECLARE _parameters json[] := '{}';
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

IF _winter_season_start IS NOT NULL THEN
    _parameters := _parameters || json_build_object('key', 'downloader.winter-season.start',
                                                    'site_id', _id,
                                                    'value', NULLIF(to_char(_winter_season_start :: date, 'MMddYYYY'), ''));
END IF;
IF _winter_season_end IS NOT NULL THEN
    _parameters := _parameters || json_build_object('key', 'downloader.winter-season.end',
                                                    'site_id', _id,
                                                    'value', NULLIF(to_char(_winter_season_end :: date, 'MMddYYYY'), ''));
END IF;
IF _summer_season_start IS NOT NULL THEN
    _parameters := _parameters || json_build_object('key', 'downloader.summer-season.start',
                                                    'site_id', _id,
                                                    'value', NULLIF(to_char(_summer_season_start :: date, 'MMddYYYY'), ''));
END IF;
IF _summer_season_end IS NOT NULL THEN
    _parameters := _parameters || json_build_object('key', 'downloader.summer-season.end',
                                                    'site_id', _id,
                                                    'value', NULLIF(to_char(_summer_season_end :: date, 'MMddYYYY'), ''));
END IF;

if array_length(_parameters, 1) > 0 THEN
    PERFORM sp_upsert_parameters(
                array_to_json(_parameters),
                false);
END IF;

END;
$BODY$
  LANGUAGE plpgsql
