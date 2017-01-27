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
DECLARE return_id smallint;
DECLARE startDateS date;
DECLARE startDateW date;
DECLARE endDateS date;
DECLARE endDateW date;
DECLARE startDate date;
DECLARE endDate date;
DECLARE midDate date;
BEGIN

    _short_name := lower(_name);
    _short_name := regexp_replace(_short_name, '\W+', '_', 'g');
    _short_name := regexp_replace(_short_name, '_+', '_', 'g');
    _short_name := regexp_replace(_short_name, '^_', '');
    _short_name := regexp_replace(_short_name, '_$', '');

	INSERT INTO site (name, short_name, geog, enabled)
		VALUES (_name, _short_name, ST_Multi(ST_Force2D(ST_GeometryFromText(_geog))) :: geography, _enabled)
	RETURNING id INTO return_id;

	IF (_winter_season_start <> '' AND _winter_season_start IS NOT NULL) THEN
        startDateW := _winter_season_start :: date;
        endDateW := _winter_season_end :: date;
		INSERT INTO config (key, site_id, value)
			VALUES ('downloader.winter-season.start', return_id, to_char(startDateW, 'MMddYYYY'));
		INSERT INTO config (key, site_id, value)
            VALUES ('downloader.winter-season.end', return_id, to_char(endDateW, 'MMddYYYY'));
	END IF;

	IF (_summer_season_start <> '' AND _summer_season_start IS NOT NULL) THEN
        startDateS := _summer_season_start :: date;
        endDateS := _summer_season_end :: date;
		INSERT INTO config (key, site_id, value)
			VALUES ('downloader.summer-season.start', return_id, to_char(startDateS, 'MMddYYYY'));
		INSERT INTO config (key, site_id, value)
            VALUES ('downloader.summer-season.end', return_id, to_char(endDateS, 'MMddYYYY'));
	END IF;

    -- what?
	startDate := CASE WHEN abs(CURRENT_DATE - startDateW) < abs(startDateS - CURRENT_DATE) THEN startDateW ELSE startDateS END;
	IF (startDate = startDateS) THEN
		endDate = endDateS;
	ELSE
		endDate = endDateW;
	END IF;
	midDate := startDate + (endDate - startDate) / 2;

	PERFORM sp_insert_scheduled_task(_short_name || '_L3A' :: character varying, 2, return_id :: int, 2::smallint, 0::smallint, 31::smallint, CAST((SELECT date_trunc('month', startDate) + interval '1 month' - interval '1 day') AS character varying), 60, 1 :: smallint, '{}' :: json);
	PERFORM sp_insert_scheduled_task(_short_name || '_L3B' :: character varying, 3, return_id :: int, 1::smallint, 10::smallint, 0::smallint, CAST((startDate + 10) AS character varying), 60, 1 :: smallint, '{"general_params":{"product_type":"L3B"}}' :: json);
	PERFORM sp_insert_scheduled_task(_short_name || '_L4A' :: character varying, 5, return_id :: int, 2::smallint, 0::smallint, 31::smallint, CAST((midDate) AS character varying), 60, 1 :: smallint, '{}' :: json);
	PERFORM sp_insert_scheduled_task(_short_name || '_L4B' :: character varying, 6, return_id :: int, 2::smallint, 0::smallint, 31::smallint, CAST((midDate) AS character varying), 60, 1 :: smallint, '{}' :: json);

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
