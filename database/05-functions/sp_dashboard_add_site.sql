CREATE OR REPLACE FUNCTION sp_dashboard_add_site(
    _name character varying,
    _geog character varying,
    _winter_season_start character varying,
    _winter_season_end character varying,
    _summer_season_start character varying,
    _summer_season_end character varying,
    _enabled boolean)
  RETURNS void AS
$BODY$
DECLARE _short_name site.short_name%TYPE;
DECLARE return_id INT;
BEGIN

    _short_name := lower(_name);
    _short_name := regexp_replace(_short_name, '\W+', '_', 'g');
    _short_name := regexp_replace(_short_name, '_+', '_', 'g');
    _short_name := regexp_replace(_short_name, '^_', '');
    _short_name := regexp_replace(_short_name, '_$', '');

	INSERT INTO site(name,short_name, geog, enabled)
	VALUES (_name,_short_name, ST_Multi(ST_Force2D(ST_GeometryFromText(_geog))) :: geography, _enabled)
	RETURNING id INTO return_id;

	IF(_winter_season_start <>''AND _winter_season_start IS NOT NULL) THEN
	INSERT INTO config(key,site_id,value)
	VALUES ('downloader.winter-season.start',return_id,_winter_season_start);
	INSERT INTO config(key,site_id,value)
	VALUES ('downloader.winter-season.end',return_id,_winter_season_end);

END IF;

	IF(_summer_season_start <>'' AND _summer_season_start IS NOT NULL)THEN
	INSERT INTO config(key,site_id,value)
	VALUES ('downloader.summer-season.start',return_id,_summer_season_start);
	INSERT INTO config(key,site_id,value)
	VALUES ('downloader.summer-season.end',return_id,_summer_season_end);
	END IF;



END;
$BODY$
  LANGUAGE plpgsql
