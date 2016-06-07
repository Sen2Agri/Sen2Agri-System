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
BEGIN

UPDATE site
SET short_name = _short_name,
    geog = ST_Multi(ST_Force2D(ST_GeometryFromText(_geog))) :: geography,
    enabled = _enabled
WHERE id = _id;

UPDATE config
SET value =_winter_season_start
WHERE key = 'downloader.winter-season.start' AND site_id = _id;
IF NOT FOUND THEN
INSERT INTO config(key,site_id,value)
		VALUES ('downloader.winter-season.start',_id,_winter_season_start);
		END IF;

UPDATE config
SET value =_winter_season_end
WHERE key = 'downloader.winter-season.end' AND site_id = _id;
IF NOT FOUND THEN
INSERT INTO config(key,site_id,value)
		VALUES ('downloader.winter-season.end',_id,_winter_season_end);
		END IF;

UPDATE config
SET value =_summer_season_start
WHERE key = 'downloader.summer-season.start' AND site_id = _id;
IF NOT FOUND THEN
INSERT INTO config(key,site_id,value)
		VALUES ('downloader.summer-season.start',_id,_summer_season_start);
END IF;

UPDATE config
SET value =_summer_season_end
WHERE key = 'downloader.summer-season.end' AND site_id = _id;
IF NOT FOUND THEN
INSERT INTO config(key,site_id,value)
		VALUES ('downloader.summer-season.end',_id,_summer_season_end);
		END IF;

END;
$BODY$
  LANGUAGE plpgsql
