begin transaction;

do $migration$
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.4') then
            raise notice 'upgrading from 1.4 to 1.4.1';

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

                raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''processor.l3b.lai.link_l3c_to_l3b'', NULL, ''0'', ''2016-02-29 14:08:07.963143+02'');';
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.link_l3c_to_l3b', NULL, '0', '2016-02-29 14:08:07.963143+02');
            end if;
            if not exists (select * from config_metadata where key = 'processor.l3b.lai.global_bv_samples_file') then
                raise notice 'INSERT INTO config_metadata VALUES (''processor.l3b.lai.global_bv_samples_file'', ''Common LAI BV sample distribution file'', ''file'', true, 4);';
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.global_bv_samples_file', 'Common LAI BV sample distribution file', 'file', true, 4);

                raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''processor.l3b.lai.global_bv_samples_file'', NULL, ''/usr/share/sen2agri/LaiCommonBVDistributionSamples.txt'', ''2016-02-29 14:08:07.963143+02'');';
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.global_bv_samples_file', NULL, '/usr/share/sen2agri/LaiCommonBVDistributionSamples.txt', '2016-02-29 14:08:07.963143+02');
            end if;

            raise notice 'update meta set version = ''1.4.1'';';
            update meta set version = '1.4.1';
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
