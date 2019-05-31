begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.8.3') then
            raise notice 'upgrading from 1.8.3 to 2.0';

            _statement := $str$
                create table if not exists l1_tile_history(
                    satellite_id smallint not null references satellite(id),
                    orbit_id int not null,
                    tile_id text not null,
                    downloader_history_id int not null references downloader_history(id),
                    status_id int not null references l1_tile_status(id),
                    status_timestamp timestamp with time zone not null default now(),
                    retry_count int not null default 0,
                    failed_reason text,
                    cloud_coverage int,
                    snow_coverage int,
                    primary key (downloader_history_id, tile_id)
                );
            $str$;
            raise notice '%', _statement;
            execute _statement;

            if not exists (with data_type_unnest as (select unnest(enum_range(NULL::t_data_type))::text as enum_values) select * from data_type_unnest where enum_values = 'select') then 
                _statement := $str$
                INSERT INTO pg_enum (enumtypid, enumlabel, enumsortorder) SELECT 't_data_type'::regtype::oid, 'select', ( SELECT MAX(enumsortorder) + 1 FROM pg_enum WHERE enumtypid = 't_data_type'::regtype );
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='processor' and column_name='label') then
                _statement := $str$
                ALTER TABLE processor ADD COLUMN label character varying null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='l1_tile_history' and column_name='cloud_coverage') then
                _statement := $str$
                ALTER TABLE l1_tile_history ADD COLUMN cloud_coverage int null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='l1_tile_history' and column_name='snow_coverage') then
                _statement := $str$
                ALTER TABLE l1_tile_history ADD COLUMN snow_coverage int null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='downloader_history' and column_name='orbit_type_id') then
                _statement := $str$
                ALTER TABLE downloader_history ADD COLUMN orbit_type_id int null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            -- datasource
            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='datasource' and column_name='site_id') then
                _statement := $str$
                ALTER TABLE datasource ADD COLUMN site_id smallint null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='datasource' and column_name='secondary_datasource_id') then
                _statement := $str$
                ALTER TABLE datasource ADD COLUMN secondary_datasource_id smallint null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            -- datasource constraints
            _statement := $str$
                ALTER TABLE datasource DROP CONSTRAINT IF EXISTS datasource_site_id_fkey;
                ALTER TABLE datasource ADD CONSTRAINT datasource_site_id_fkey FOREIGN KEY (site_id) REFERENCES site (id) MATCH SIMPLE ON UPDATE NO ACTION ON DELETE NO ACTION;
            $str$;
            raise notice '%', _statement;
            execute _statement;
      
            _statement := $str$
                ALTER TABLE datasource DROP CONSTRAINT IF EXISTS fk_datasource_datasource;
                ALTER TABLE datasource ADD CONSTRAINT fk_datasource_datasource FOREIGN KEY (secondary_datasource_id) REFERENCES datasource (id) MATCH SIMPLE ON UPDATE NO ACTION ON DELETE NO ACTION;
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            UPDATE datasource SET specific_params = '{"parameters":{"dsParameter":[{"name":"minClouds","type":"java.lang.Integer","value":"0"},{"name":"maxClouds","type":"java.lang.Integer","value":"100"},{"name":"platformName","type":"java.lang.String","value":"LANDSAT_8_C1"}]}}'
            WHERE name = 'USGS';
            
            UPDATE datasource SET specific_params = '{"parameters":{"dsParameter":[{"name":"platformName","type":"java.lang.String","value":"Sentinel-2"},{"name":"productType","type":"java.lang.String","value":"S2MSI1C"}]}}'
            WHERE name = 'Scientific Data Hub' and satellite_id = 1;            
            -- product
            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='product' and column_name='orbit_type_id') then
                _statement := $str$
                ALTER TABLE product ADD COLUMN orbit_type_id smallint null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='product' and column_name='downloader_history_id') then
                _statement := $str$
                ALTER TABLE product ADD COLUMN downloader_history_id int null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            -- orbit_type
            _statement := $str$
                CREATE TABLE IF NOT EXISTS orbit_type
                (
                  id smallint NOT NULL,
                  description text NOT NULL,
                  CONSTRAINT orbit_type_pkey PRIMARY KEY (id)
                );
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            -- FUNCTIONS
            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_insert_product(
                    _product_type_id smallint,
                    _processor_id smallint,
                    _satellite_id integer,
                    _site_id smallint,
                    _job_id integer,
                    _full_path character varying,
                    _created_timestamp timestamp with time zone,
                    _name character varying,
                    _quicklook_image character varying,
                    _footprint geography,
                    _orbit_id integer,
                    _tiles json,
                    _orbit_type_id smallint DEFAULT NULL::smallint,
                    _downloader_history_id integer DEFAULT NULL::integer)
                  RETURNS integer AS
                $BODY$
                DECLARE return_id product.id%TYPE;
                BEGIN
                    UPDATE product
                    SET job_id = _job_id,
                        full_path = _full_path,
                        created_timestamp = _created_timestamp,
                        quicklook_image = _quicklook_image,
                        footprint = (SELECT '(' || string_agg(REPLACE(replace(ST_AsText(geom) :: text, 'POINT', ''), ' ', ','), ',') || ')'
                                     from ST_DumpPoints(ST_Envelope(_footprint :: geometry))
                                     WHERE path[2] IN (1, 3)) :: POLYGON,
                        geog = _footprint,
                        tiles = array(select tile :: character varying from json_array_elements_text(_tiles) tile),
                        is_archived = FALSE
                    WHERE product_type_id = _product_type_id
                      AND processor_id = _processor_id
                      AND satellite_id = _satellite_id
                      AND site_id = _site_id
                      AND COALESCE(orbit_id, 0) = COALESCE(_orbit_id, 0)
                      AND "name" = _name
                    RETURNING id INTO return_id;

                    IF NOT FOUND THEN
                        INSERT INTO product(
                            product_type_id,
                            processor_id,
                            satellite_id,
                            job_id,
                            site_id,
                            full_path,
                            created_timestamp,
                            "name",
                            quicklook_image,
                            footprint,
                            geog,
                            orbit_id,
                            tiles,
                            orbit_type_id,
                            downloader_history_id
                        )
                        VALUES (
                            _product_type_id,
                            _processor_id,
                            _satellite_id,
                            _job_id,
                            _site_id,
                            _full_path,
                            COALESCE(_created_timestamp, now()),
                            _name,
                            _quicklook_image,
                            (SELECT '(' || string_agg(REPLACE(replace(ST_AsText(geom) :: text, 'POINT', ''), ' ', ','), ',') || ')'
                             from ST_DumpPoints(ST_Envelope(_footprint :: geometry))
                             WHERE path[2] IN (1, 3)) :: POLYGON,
                             _footprint,
                             _orbit_id,
                            array(select tile :: character varying from json_array_elements_text(_tiles) tile),
                            _orbit_type_id,
                            _downloader_history_id
                        )
                        RETURNING id INTO return_id;
                    END IF;

                    RETURN return_id;
                END;
                $BODY$
                  LANGUAGE plpgsql VOLATILE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$            
                create or replace function sp_mark_l1_tile_done(
                    _downloader_history_id int,
                    _tile_id text,
                    _cloud_coverage int,
                    _snow_coverage int
                )
                returns boolean
                as
                $$
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    update l1_tile_history
                    set status_id = 3, -- done
                        status_timestamp = now(),
                        failed_reason = null,
                        cloud_coverage = _cloud_coverage,
                        snow_coverage = _snow_coverage
                    where (downloader_history_id, tile_id) = (_downloader_history_id, _tile_id);

                    return sp_update_l1_tile_status(_downloader_history_id);
                end;
                $$ language plpgsql volatile;            
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$    
                create or replace function sp_mark_l1_tile_failed(
                    _downloader_history_id int,
                    _tile_id text,
                    _reason text,
                    _should_retry boolean,
                    _cloud_coverage int,
                    _snow_coverage int
                )
                returns boolean
                as
                $$
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    update l1_tile_history
                    set status_id = 2, -- failed
                        status_timestamp = now(),
                        retry_count = case _should_retry
                            when true then retry_count + 1
                            else 3
                        end,
                        failed_reason = _reason,
                        cloud_coverage = _cloud_coverage,
                        snow_coverage = _snow_coverage
                    where (downloader_history_id, tile_id) = (_downloader_history_id, _tile_id);

                    return sp_update_l1_tile_status(_downloader_history_id);
                end;
                $$ language plpgsql volatile;            
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$               
                create or replace function sp_start_l1_tile_processing()
                returns table (
                    site_id int,
                    satellite_id smallint,
                    orbit_id int,
                    tile_id text,
                    downloader_history_id int,
                    path text,
                    prev_l2a_path text
                ) as
                $$
                declare _satellite_id smallint;
                declare _orbit_id int;
                declare _tile_id text;
                declare _downloader_history_id int;
                declare _path text;
                declare _prev_l2a_path text;
                declare _site_id int;
                declare _product_date timestamp;
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    select l1_tile_history.satellite_id,
                           l1_tile_history.orbit_id,
                           l1_tile_history.tile_id,
                           l1_tile_history.downloader_history_id
                    into _satellite_id,
                         _orbit_id,
                         _tile_id,
                         _downloader_history_id
                    from l1_tile_history
                    where status_id = 2 -- failed
                      and retry_count < 3
                      and status_timestamp < now() - interval '1 day'
                    order by status_timestamp
                    limit 1;

                    if found then
                        select downloader_history.product_date,
                               downloader_history.full_path,
                               downloader_history.site_id
                        into _product_date,
                             _path,
                             _site_id
                        from downloader_history
                        where id = _downloader_history_id;

                        update l1_tile_history
                        set status_id = 1, -- processing
                            status_timestamp = now()
                        where (l1_tile_history.downloader_history_id, l1_tile_history.tile_id) = (_downloader_history_id, _tile_id);
                    else
                        select distinct
                            downloader_history.satellite_id,
                            downloader_history.orbit_id,
                            tile_ids.tile_id,
                            downloader_history.id,
                            downloader_history.product_date,
                            downloader_history.full_path,
                            downloader_history.site_id
                        into _satellite_id,
                            _orbit_id,
                            _tile_id,
                            _downloader_history_id,
                            _product_date,
                            _path,
                            _site_id
                        from downloader_history
                        cross join lateral (
                                select unnest(tiles) as tile_id
                            ) tile_ids
                        inner join site on site.id = downloader_history.site_id
                        where not exists (
                            select *
                            from l1_tile_history
                            where (l1_tile_history.satellite_id,
                                   l1_tile_history.orbit_id,
                                   l1_tile_history.tile_id) =
                                  (downloader_history.satellite_id,
                                   downloader_history.orbit_id,
                                   tile_ids.tile_id)
                              and (status_id = 1 or -- processing
                                   retry_count < 3 and status_id = 2 -- failed
                              )
                              or (l1_tile_history.downloader_history_id, l1_tile_history.tile_id) = (downloader_history.id, tile_ids.tile_id)
                        ) and downloader_history.status_id in (2, 7) -- downloaded, processing
                        and site.enabled
                        and downloader_history.satellite_id in (1, 2) -- sentinel2, landsat8
                        order by satellite_id,
                                orbit_id,
                                tile_id,
                                product_date
                        limit 1;

                        if found then
                            insert into l1_tile_history (
                                satellite_id,
                                orbit_id,
                                tile_id,
                                downloader_history_id,
                                status_id
                            ) values (
                                _satellite_id,
                                _orbit_id,
                                _tile_id,
                                _downloader_history_id,
                                1 -- processing
                            );

                            update downloader_history
                            set status_id = 7 -- processing
                            where id = _downloader_history_id;
                        end if;
                    end if;

                    if _downloader_history_id is not null then
                        select product.full_path
                        into _prev_l2a_path
                        from product
                        where product.site_id = _site_id
                          and product.product_type_id = 1 -- l2a
                          and product.satellite_id = _satellite_id
                          and product.created_timestamp < _product_date
                          and product.tiles :: text[] @> array[_tile_id]
                          and (product.satellite_id <> 1 -- sentinel2
                               or product.orbit_id = _orbit_id)
                        order by created_timestamp desc
                        limit 1;

                        return query
                            select _site_id,
                                _satellite_id,
                                _orbit_id,
                                _tile_id,
                                _downloader_history_id,
                                _path,
                                _prev_l2a_path;
                    end if;
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                create or replace function sp_clear_pending_l1_tiles()
                    returns void
                    as
                    $$
                    begin
                        delete
                        from l1_tile_history
                        where status_id = 1; -- processing

                        update downloader_history
                        set status_id = 2 -- downloaded
                        where status_id = 7 -- processing
                          and not exists (
                            select *
                            from l1_tile_history
                            where status_id = 1 -- processing
                        );
                    end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;
           
            UPDATE processor SET label = 'L2A &mdash; Atmospheric Corrections' WHERE id = 1;
            UPDATE processor SET label = 'L3A &mdash; Cloud-free Composite' WHERE id = 2;
            UPDATE processor SET label = 'L3B &mdash; LAI/NDVI' WHERE id = 3;
            UPDATE processor SET label = 'L3E &mdash; Phenology Indices' WHERE id = 4;
            UPDATE processor SET label = 'L4A &mdash; Cropland Mask' WHERE id = 5;
            UPDATE processor SET label = 'L4B &mdash; Crop Type Map' WHERE id = 6;

            UPDATE config SET value = 8082 WHERE key = 'http-listener.listen-port';
            
            UPDATE config_metadata SET type = 'file' WHERE key = 'executor.module.path.gdal_translate';
            UPDATE config_metadata SET type = 'file' WHERE key = 'executor.module.path.gdalbuildvrt';
            
            -- Update for fields used in visualization
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Minimum weight depending on AOT'  where key = 'processor.l3a.weight.aot.minweight';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Maximum weight depending on AOT'  where key = 'processor.l3a.weight.aot.maxweight';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Coarse resolution for quicker convolution'  where key = 'processor.l3a.weight.cloud.coarseresolution';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Maximum value of the linear range for weights w.r.t. AOT'  where key = 'processor.l3a.weight.aot.maxaot';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Standard deviation of gaussian filter for distance to large clouds'  where key = 'processor.l3a.weight.cloud.sigmasmall';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Standard deviation of gaussian filter for distance to small clouds'  where key = 'processor.l3a.weight.cloud.sigmalarge';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Minimum weight at edge of the synthesis time window'  where key = 'processor.l3a.weight.total.weightdatemin';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Generate Models'  where key = 'processor.l3b.generate_models';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Number of components'  where key = 'processor.l4a.nbcomp';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Range radius'  where key = 'processor.l4a.range-radius';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Erosion radius'  where key = 'processor.l4a.erode-radius';
            update config_metadata set is_advanced = true, is_site_visible = true, 	label = 'Mahalanobis alpha'  where key = 'processor.l4a.mahalanobis-alpha';
            update config_metadata set is_advanced = true, is_site_visible = true,  values = '{"min":"0", "step":"1", "max":"", "type":"classifier"}',label = 'The minium number of pixels'  where key = 'processor.l4a.min-area';
            update config_metadata set is_advanced = true, is_site_visible = true,  values = '{"min":"0", "step":"1", "max":"", "type":"classifier"}',label = 'Training trees'  where key = 'processor.l4a.classifier.rf.nbtrees';
            update config_metadata set is_advanced = true, is_site_visible = true,  values = '{"min":"0", "step":"1", "max":"", "type":"classifier"}',label = 'Max depth'  where key = 'processor.l4a.classifier.rf.max';
            update config_metadata set is_advanced = true, is_site_visible = true,  values = '{"min":"0", "step":"1", "max":"", "type":"classifier"}',label = 'Minimum number of samples'  where key = 'processor.l4a.classifier.rf.min';
            update config_metadata set is_advanced = true, is_site_visible = true,  label = 'Ratio'  where key = 'processor.l4a.sample-ratio';
            update config_metadata set is_advanced = true, is_site_visible = true,  values = '{"min":"0", "step":"1", "max":"", "type":"classifier"}',label = 'Random Forest classifier max depth'  where key = 'processor.l4b.classifier.rf.max';
            update config_metadata set is_advanced = true, is_site_visible = true,  values = '{"min":"0", "step":"1", "max":"", "type":"classifier"}',label = 'Minimum number of samples'  where key = 'processor.l4b.classifier.rf.min';
            update config_metadata set is_advanced = true, is_site_visible = true,  values = '{"min":"0", "step":"1", "max":"", "type":"classifier"}',label = 'Training trees'  where key = 'processor.l4b.classifier.rf.nbtrees';
            update config_metadata set is_advanced = true, is_site_visible = true,  label = 'Random seed'  where key = 'processor.l4b.random_seed';
            
            if not exists (select * from config where key = 'processor.l4b.crop-mask') then
                _statement := $str$
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4b.crop-mask', NULL, '', '2019-04-12 14:56:57.501918+02');
                    INSERT INTO config_metadata VALUES ('processor.l4b.crop-mask', 'Crop mask file path or product folder to be used', 'file', false, 6, true, 'Crop masks', '{"name":"cropMask"}');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;                
                
            -- End of Update for fields used in visualization
            
            if not exists (select * from orbit_type) then
                _statement := $str$
                    INSERT INTO orbit_type (id, description) VALUES (1, 'ASCENDING');
                    INSERT INTO orbit_type (id, description) VALUES (2, 'DESCENDING');            
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;                

            _statement := $str$
                ALTER TABLE downloader_history DROP CONSTRAINT IF EXISTS fk_product_orbit_type;
                ALTER TABLE downloader_history ADD CONSTRAINT fk_product_orbit_type FOREIGN KEY (orbit_type_id) REFERENCES public.orbit_type (id);
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                ALTER TABLE product DROP CONSTRAINT IF EXISTS fk_product_orbit_type;
                ALTER TABLE product ADD CONSTRAINT fk_product_orbit_type FOREIGN KEY (orbit_type_id) REFERENCES public.orbit_type (id);
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            if not exists (select * from config where key = 'downloader.s2.forcestart') then
                _statement := $str$
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.forcestart', NULL, 'false', '2019-04-12 14:56:57.501918+02');
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.forcestart', NULL, 'false', '2019-04-12 14:56:57.501918+02');
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s1.forcestart', NULL, 'false', '2019-04-12 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.skip.existing' and site_id is null) then
                _statement := $str$
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.skip.existing', NULL, 'false', '2019-04-12 14:56:57.501918+02');
                    INSERT INTO config_metadata VALUES ('downloader.skip.existing', 'If enabled, products downloaded for another site will be duplicated, in database only, for the current site', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;                

            if not exists (select * from config_metadata where key = 'downloader.s2.forcestart') then
                _statement := $str$
                    INSERT INTO config_metadata VALUES ('downloader.s2.forcestart', 'Forces the S2 download to start again from the beginning of the season', 'bool', false, 15);
                    INSERT INTO config_metadata VALUES ('downloader.l8.forcestart', 'Forces the L8 download to start again from the beginning of the season', 'bool', false, 15);
                    INSERT INTO config_metadata VALUES ('downloader.s1.forcestart', 'Forces the S1 download to start again from the beginning of the season', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            
            _statement := 'update meta set version = ''2.0'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
