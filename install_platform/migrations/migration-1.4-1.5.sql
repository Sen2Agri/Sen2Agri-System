begin transaction;

do $migration$
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.4') then
            raise notice 'upgrading from 1.4 to 1.5';

            raise notice 'applying b37a2c5133daf54b44043e9476224d4b9539bdd8';
            raise notice 'CREATE OR REPLACE FUNCTION sp_get_intersecting_tiles(INTEGER, TEXT) [...];';
            CREATE OR REPLACE FUNCTION sp_get_intersecting_tiles(
                _satellite_id satellite.id%TYPE,
                _tile_id TEXT
            )
            RETURNS TABLE (
                satellite_id satellite.id%TYPE,
                tile_id TEXT
            )
            AS $$
            DECLARE _geog GEOGRAPHY;
            BEGIN
                CASE _satellite_id
                    WHEN 1 THEN
                    BEGIN
                        _geog := (SELECT geog
                                  FROM shape_tiles_s2
                                  WHERE shape_tiles_s2.tile_id = _tile_id);
                    END;
                    WHEN 2 THEN
                    BEGIN
                        _geog := (SELECT geog
                                  FROM shape_tiles_l8
                                  WHERE pr = _tile_id :: INT);
                    END;
                END CASE;

                RETURN QUERY
                    SELECT 1 AS satellite_id,
                        shape_tiles_s2.tile_id :: TEXT
                    FROM shape_tiles_s2
                    WHERE ST_Intersects(shape_tiles_s2.geog, _geog)
                    UNION
                    SELECT 2 AS satellite_id,
                        lpad(shape_tiles_l8.pr :: TEXT, 6, '0')
                    FROM shape_tiles_l8
                    WHERE ST_Intersects(shape_tiles_l8.geog, _geog)
                    ORDER BY satellite_id, tile_id;
            END;
            $$
            LANGUAGE plpgsql
            STABLE;

            -- b60935d8d4e456b82d2799500254316f97010449
            -- skipped, no functional changes

            raise notice 'applying 0a7b69dc67cbc32db347ff6e2be0cc3688ef5df1';
            if not exists (select * from config_metadata where key = 'processor.l3b.lai.link_l3c_to_l3b') then
                raise notice 'INSERT INTO config_metadata VALUES (''processor.l3b.lai.link_l3c_to_l3b'', ''Trigger an L3C product creation after L3B product creation'', ''int'', true, 4);';
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.link_l3c_to_l3b', 'Trigger an L3C product creation after L3B product creation', 'int', true, 4);
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.link_l3c_to_l3b' and site_id is null) then
                raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''processor.l3b.lai.link_l3c_to_l3b'', NULL, ''0'', ''2016-02-29 14:08:07.963143+02'');';
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.link_l3c_to_l3b', NULL, '0', '2016-02-29 14:08:07.963143+02');
            end if;
            if not exists (select * from config_metadata where key = 'processor.l3b.lai.global_bv_samples_file') then
                raise notice 'INSERT INTO config_metadata VALUES (''processor.l3b.lai.global_bv_samples_file'', ''Common LAI BV sample distribution file'', ''file'', true, 4);';
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.global_bv_samples_file', 'Common LAI BV sample distribution file', 'file', true, 4);
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.global_bv_samples_file' AND site_id is null) then
                raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''processor.l3b.lai.global_bv_samples_file'', NULL, ''/usr/share/sen2agri/LaiCommonBVDistributionSamples.txt'', ''2016-02-29 14:08:07.963143+02'');';
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.global_bv_samples_file', NULL, '/usr/share/sen2agri/LaiCommonBVDistributionSamples.txt', '2016-02-29 14:08:07.963143+02');
            end if;

            -- 0d84cd4419eb22af2bcf0a56888e8b10d952be30
            -- skipped, superseeded by 1005e8dc54459841ce041572b84bad29b44562f5

            raise notice 'applying 615dc5d8322fcbb99dd5b340f1fda881d5580d92';
            raise notice 'CREATE OR REPLACE FUNCTION sp_get_product_by_name(SMALLINT, CHARACTER VARYING) [...];';
            CREATE OR REPLACE FUNCTION sp_get_product_by_name(
                _site_id site.id%TYPE,
                _name character varying)
            RETURNS TABLE(product_id smallint, product_type_id smallint, processor_id smallint, site_id smallint, full_path character varying, created_timestamp timestamp with time zone) AS
            $BODY$
            BEGIN

            RETURN QUERY SELECT product.product_type_id AS product_id, product.product_type_id, product.processor_id, product.site_id, product.full_path, product.created_timestamp
            FROM product
            WHERE product.site_id = _site_id AND
                product.name = _name;

            END;
            $BODY$
            LANGUAGE plpgsql VOLATILE
            COST 100
            ROWS 1000;
            ALTER FUNCTION sp_get_product_by_name(character varying)
            OWNER TO admin;

            raise notice 'applying 1005e8dc54459841ce041572b84bad29b44562f5';
            raise notice 'CREATE OR REPLACE FUNCTION sp_get_products_for_tile(INTEGER, CHARACTER VARYING, SMALLINT, INT, INT) [...];';
            CREATE OR REPLACE FUNCTION sp_get_products_for_tile(_site_id site.id%TYPE, _tile_id CHARACTER VARYING, _product_type_id SMALLINT, _satellite_id satellite.id%TYPE, _out_satellite_id satellite.id%TYPE)
            RETURNS TABLE (
                full_path product.full_path%TYPE,
                product_date product.created_timestamp%TYPE
            )
            AS $$
            DECLARE _geog GEOGRAPHY;
            BEGIN
                CASE _satellite_id
                    WHEN 1 THEN -- sentinel2
                        _geog := (SELECT shape_tiles_s2.geog FROM shape_tiles_s2 WHERE tile_id = _tile_id);
                    WHEN 2 THEN -- landsat8
                        _geog := (SELECT shape_tiles_l8 FROM shape_tiles_l8 WHERE shape_tiles_l8.pr = _tile_id :: INT);
                END CASE;

                RETURN QUERY
                    SELECT product.full_path,
                        product.created_timestamp
                    FROM product
                    WHERE product.site_id = _site_id AND
                        product.satellite_id = _out_satellite_id AND
                        product.product_type_id = _product_type_id AND
                        ST_Intersects(product.geog, _geog);
            END;
            $$
            LANGUAGE plpgsql
            STABLE;

            raise notice 'applying 1ba22372e238be457de560d46170687c461af810';
            raise notice 'CREATE OR REPLACE FUNCTION sp_dashboard_add_site(character varying, character varying, character varying, character varying, character varying, character varying, boolean) [...];';
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

            raise notice 'applying f2acd7c5d682283e8ae5736fce78a3a63b9c1bc5';
            raise notice 'CREATE OR REPLACE FUNCTION sp_dashboard_update_site(smallint, character varying, character varying, character varying, character varying, character varying, character varying, boolean) [...];';
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
                                                                'value', NULLIF(to_char(nullif(_winter_season_start, '') :: date, 'MMddYYYY'), ''));
            END IF;
            IF _winter_season_end IS NOT NULL THEN
                _parameters := _parameters || json_build_object('key', 'downloader.winter-season.end',
                                                                'site_id', _id,
                                                                'value', NULLIF(to_char(nullif(_winter_season_end, '') :: date, 'MMddYYYY'), ''));
            END IF;
            IF _summer_season_start IS NOT NULL THEN
                _parameters := _parameters || json_build_object('key', 'downloader.summer-season.start',
                                                                'site_id', _id,
                                                                'value', NULLIF(to_char(nullif(_summer_season_start, '') :: date, 'MMddYYYY'), ''));
            END IF;
            IF _summer_season_end IS NOT NULL THEN
                _parameters := _parameters || json_build_object('key', 'downloader.summer-season.end',
                                                                'site_id', _id,
                                                                'value', NULLIF(to_char(nullif(_summer_season_end, '') :: date, 'MMddYYYY'), ''));
            END IF;

            if array_length(_parameters, 1) > 0 THEN
                PERFORM sp_upsert_parameters(
                            array_to_json(_parameters),
                            false);
            END IF;

            END;
            $BODY$
            LANGUAGE plpgsql;

            raise notice 'applying b0041d66fde28b7b78a22db57bde3d99d4885576';
            raise notice 'CREATE OR REPLACE FUNCTION sp_get_dashboard_products(smallint, smallint) [...];';
            CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
                _site_id smallint DEFAULT NULL::smallint,
                _processor_id smallint DEFAULT NULL::smallint)
            RETURNS TABLE (
            --     id product.id%type,
            --     satellite_id product.satellite_id%type,
            --     product product.name%type,
            --     product_type product_type.name%type,
            --     product_type_description product_type.description%type,
            --     processor processor.name%type,
            --     site site.name%type,
            --     full_path product.full_path%type,
            --     quicklook_image product.quicklook_image%type,
            --     footprint product.footprint%type,
            --     created_timestamp product.created_timestamp%type,
            --     site_coord text
                json json
            ) AS
            $BODY$
            DECLARE summer_season_start_str text;
            DECLARE summer_season_end_str text;
            DECLARE winter_season_start_str text;
            DECLARE winter_season_end_str text;
            DECLARE summer_season_start date;
            DECLARE summer_season_end date;
            DECLARE winter_season_start date;
            DECLARE winter_season_end date;
            DECLARE q text;
            BEGIN
                q := $sql$
                    WITH site_names(id, name, geog, row) AS (
                            select id, name, st_astext(geog), row_number() over (order by name)
                            from site
                        ),
                        product_type_names(id, name, description, row) AS (
                            select id, name, description, row_number() over (order by description)
                            from product_type
                        ),
                        data(id, satellite_id, product,product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
                        SELECT
                            P.id,
                            P.satellite_id,
                            P.name,
                            PT.name,
                            PT.description,
                            PR.name,
                            S.name,
                            P.full_path,
                            P.quicklook_image,
                            P.footprint,
                            P.created_timestamp,
                            S.geog
                        FROM product P
                            JOIN product_type_names PT ON P.product_type_id = PT.id
                            JOIN processor PR ON P.processor_id = PR.id
                            JOIN site_names S ON P.site_id = S.id
                        WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
                $sql$;
                IF $1 IS NOT NULL THEN
                    q := q || 'AND P.site_id = $1';

                    summer_season_start_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.summer-season.start');
                    summer_season_end_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.summer-season.end');
                    winter_season_start_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.winter-season.start');
                    winter_season_end_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.winter-season.end');

                    if length(summer_season_start_str) = 8 and length(summer_season_end_str) = 8 then
                        summer_season_start_str := right(summer_season_start_str, 4) || left(summer_season_start_str, 4);
                        summer_season_end_str := right(summer_season_end_str, 4) || left(summer_season_end_str, 4);
                        summer_season_start := summer_season_start_str :: date;
                        summer_season_end := summer_season_end_str :: date;
                    end if;

                    if length(winter_season_start_str) = 8 and length(winter_season_end_str) = 8 then
                        winter_season_start_str := right(winter_season_start_str, 4) || left(winter_season_start_str, 4);
                        winter_season_end_str := right(winter_season_end_str, 4) || left(winter_season_end_str, 4);
                        winter_season_start := winter_season_start_str :: date;
                        winter_season_end := winter_season_end_str :: date;
                    end if;

                    if summer_season_start is not null and summer_season_end is not null or
                    winter_season_start is not null and winter_season_end is not null then
                        q := q || 'AND (FALSE';
                        if summer_season_start is not null and summer_season_end is not null then
                            q := q || ' OR P.created_timestamp BETWEEN $3 AND $4';
                        end if;
                        if winter_season_start is not null and winter_season_end is not null then
                            q := q || ' OR P.created_timestamp BETWEEN $5 AND $6';
                        end if;
                        q := q || ')';
                    end if;
                END IF;
                IF $2 IS NOT NULL THEN
                    q := q || 'AND P.product_type_id = $2';
                END IF;

                q := q || $sql$
                        ORDER BY S.row, PT.row, P.name
                    )
            --         select * from data;
                    SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
                $sql$;

            --     raise notice '%', q;

                RETURN QUERY
                EXECUTE q
                USING _site_id, _processor_id, summer_season_start, summer_season_end, winter_season_start, winter_season_end;
            END
            $BODY$
            LANGUAGE plpgsql STABLE
            COST 100;

            raise notice 'update meta set version = ''1.5'';';
            update meta set version = '1.5';
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
