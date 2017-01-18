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

            raise notice 'update meta set version = ''1.4.1'';';
            update meta set version = '1.4.1';
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
