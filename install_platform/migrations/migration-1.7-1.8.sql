begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.7') then
            raise notice 'upgrading from 1.7 to 1.8';

            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='downloader_history' and column_name='status_reason') then
                ALTER TABLE downloader_history ADD COLUMN status_reason character varying null;
            end if;

            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='downloader_history' and column_name='tiles') then
                ALTER TABLE downloader_history ADD COLUMN tiles character varying null;
            end if;
            
            if not exists (select * from config_metadata where key = 'processor.l3b.lai.use_inra_version') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.use_inra_version', 'L3B LAI processor will use INRA algorithm implementation', 'int', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.use_inra_version' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.use_inra_version', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l3b.lai.tiles_filter') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.tiles_filter', 'L3B LAI processor tiles filter', 'string', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.tiles_filter' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.tiles_filter', NULL, '', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l3b.lai.produce_ndvi') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_ndvi', 'L3B LAI processor will produce NDVI', 'int', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.produce_ndvi' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_ndvi', NULL, '1', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l3b.lai.produce_lai') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_lai', 'L3B LAI processor will produce LAI', 'int', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.produce_lai' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_lai', NULL, '1', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l3b.lai.produce_fapar') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_fapar', 'L3B LAI processor will produce FAPAR', 'int', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.produce_fapar' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_fapar', NULL, '1', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l3b.lai.produce_fcover') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_fcover', 'L3B LAI processor will produce FCOVER', 'int', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.produce_fcover' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_fcover', NULL, '1', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if exists (select * from config where key = 'processor.l4a.reference-map' and site_id is null) then
                _statement := $str$
                UPDATE config SET VALUE = '/mnt/archive/reference_data/ESACCI-LC-L4-LCCS-Map-300m-P1Y-2015-v2.0.7.tif' where key = 'processor.l4a.reference-map' and site_id is null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from downloader_status where status_description = 'processing') then
                raise notice 'INSERT INTO downloader_status VALUES (7, ''processing'');';
                INSERT INTO downloader_status VALUES (7, 'processing');
            end if;
            
            if not exists (select * from downloader_status where status_description = 'processing_cld_failed') then
                raise notice 'INSERT INTO downloader_status VALUES (8, ''processing_cld_failed'');';
                INSERT INTO downloader_status VALUES (8, 'processing_cld_failed');
            end if;

            if not exists (select * from downloader_status where status_description = 'download_ignored') then
                raise notice 'INSERT INTO downloader_status VALUES (41, ''download_ignored'');';
                INSERT INTO downloader_status VALUES (41, 'download_ignored');
            end if;
            
            _statement := $str$
            drop function if exists sp_insert_product(smallint, smallint, integer, smallint, integer, character varying, timestamp with time zone, character varying, character varying, geography, json);
            $str$;
            raise notice '%', _statement;
            execute _statement;


            if not exists (select * from config_metadata where key = 'downloader.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.enabled', 'Downloader is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s1.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s1.enabled', 'S1 downloader is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s1.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s1.enabled', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s2.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s2.enabled', 'S2 downloader is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s2.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.l8.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.l8.enabled', 'L8 downloader is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.l8.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 's1.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('s1.enabled', 'S1 is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 's1.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('s1.enabled', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 's2.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('s2.enabled', 'S2 is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 's2.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('s2.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'l8.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('l8.enabled', 'L8 is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'l8.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('l8.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'scheduled.lookup.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('scheduled.lookup.enabled', 'Scheduled lookup is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'scheduled.lookup.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.lookup.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'scheduled.retry.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('scheduled.retry.enabled', 'Scheduled retry is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'scheduled.retry.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.retry.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'scheduled.object.storage.move.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('scheduled.object.storage.move.enabled', 'Scheduled object storage move enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'scheduled.object.storage.move.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.object.storage.move.enabled', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'scheduled.object.storage.move.product.types') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('scheduled.object.storage.move.product.types', 'Product types to move to object storage (separated by ;)', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'scheduled.object.storage.move.product.types' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.object.storage.move.product.types', NULL, '', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'scheduled.object.storage.move.deleteAfter') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('scheduled.object.storage.move.deleteAfter', 'Scheduled object storage move enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'scheduled.object.storage.move.deleteAfter' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.object.storage.move.deleteAfter', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            if not exists (select * from config_metadata where key = 'processor.l4a.tile-threads-hint') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l4a.tile-threads-hint', 'Threads to use for classification of a tile', 'int', false, 5);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l4a.tile-threads-hint' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4a.tile-threads-hint', NULL, '4', '2018-01-26 17:17:52+02:00');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l4a.max-parallelism') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l4a.max-parallelism', 'Tiles to classify in parallel', 'int', false, 5);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l4a.max-parallelism' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4a.max-parallelism', NULL, '0', '2018-01-26 17:17:52+02:00');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l4b.tile-threads-hint') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l4b.tile-threads-hint', 'Threads to use for classification of a tile', 'int', false, 6);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l4b.tile-threads-hint' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4b.tile-threads-hint', NULL, '4', '2018-01-26 17:17:52+02:00');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l4b.max-parallelism') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l4b.max-parallelism', 'Tiles to classify in parallel', 'int', false, 6);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l4b.max-parallelism' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4b.max-parallelism', NULL, '0', '2018-01-26 17:17:52+02:00');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            
            if not exists (select * from config_metadata where key = 'processor.l3a.cloud_optimized_geotiff_output') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3a.cloud_optimized_geotiff_output', 'Output L3A as Cloud Optimized Geotiff', 'bool', false, 3);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3a.cloud_optimized_geotiff_output' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3a.cloud_optimized_geotiff_output', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l3b.cloud_optimized_geotiff_output') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.cloud_optimized_geotiff_output', 'Output L3B as Cloud Optimized Geotiff', 'bool', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.cloud_optimized_geotiff_output' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.cloud_optimized_geotiff_output', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l3e.cloud_optimized_geotiff_output') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3e.cloud_optimized_geotiff_output', 'Output L3E as Cloud Optimized Geotiff', 'bool', false, 18);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3e.cloud_optimized_geotiff_output' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3e.cloud_optimized_geotiff_output', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l4a.cloud_optimized_geotiff_output') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l4a.cloud_optimized_geotiff_output', 'Output L4A as Cloud Optimized Geotiff', 'bool', false, 5);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l4a.cloud_optimized_geotiff_output' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4a.cloud_optimized_geotiff_output', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'processor.l4b.cloud_optimized_geotiff_output') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l4b.cloud_optimized_geotiff_output', 'Output L4B as Cloud Optimized Geotiff', 'bool', false, 6);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l4b.cloud_optimized_geotiff_output' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4b.cloud_optimized_geotiff_output', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'demmaccs.compress-tiffs') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('demmaccs.compress-tiffs', 'Demmaccs compress output tiff files', 'bool', false, 16);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'demmaccs.compress-tiffs' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.compress-tiffs', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'demmaccs.cog-tiffs') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('demmaccs.cog-tiffs', 'Demmaccs outputs Cloud Optimized Geotiff files', 'bool', false, 16);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'demmaccs.cog-tiffs' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.cog-tiffs', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'demmaccs.remove-sre') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('demmaccs.remove-sre', 'Demmaccs remove SRE files', 'bool', false, 16);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'demmaccs.remove-sre' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.remove-sre', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'demmaccs.remove-fre') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('demmaccs.remove-fre', 'Demmaccs remove FRE files', 'bool', false, 16);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'demmaccs.remove-fre' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.remove-fre', NULL, 'false', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            _statement = 'DROP FUNCTION IF EXISTS sp_get_dashboard_downloader_history(smallint)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_dashboard_downloader_history(IN _siteid smallint DEFAULT NULL::smallint)
                  RETURNS TABLE(id integer, nr_downloads bigint) AS
                $BODY$
                BEGIN
                   RETURN QUERY

                    SELECT  1, count(status_id)
                        FROM downloader_history
                        WHERE status_id  = 1 AND ( $1 IS NULL OR site_id = _siteid)
                   UNION
                    SELECT  2, count(status_id)
                        FROM downloader_history
                        WHERE status_id IN (2, 5, 6, 7) AND ( $1 IS NULL OR site_id = _siteid)
                   UNION
                    SELECT  3, count(status_id)
                        FROM downloader_history
                        WHERE status_id IN (3, 4) AND ( $1 IS NULL OR site_id = _siteid)
                   ORDER BY 1;

                END
                $BODY$
                LANGUAGE plpgsql
                STABLE;
                $str$;
            raise notice '%', _statement;
            execute _statement;
            

            _statement = 'DROP FUNCTION IF EXISTS sp_mark_job_resumed(integer)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_mark_job_resumed(_job_id integer)
                  RETURNS void AS
                $$
                DECLARE unrunnable_task_ids int[];
                DECLARE runnable_task_ids int[];
                DECLARE runnable_task_id int;
                DECLARE processor_id processor.id%TYPE;
                BEGIN

                    IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
                        RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
                    END IF;

                    UPDATE step
                    SET status_id = 1, --Submitted
                    status_timestamp = now()
                    FROM task
                    WHERE task.id = step.task_id AND task.job_id = _job_id
                    AND step.status_id = 5; --Paused

                    -- Get the list of tasks that depended on tasks that have NOT been finished
                    SELECT array_agg(task.id) INTO unrunnable_task_ids FROM task
                    WHERE task.job_id = _job_id
                      AND task.status_id = 5 --Paused
                      AND EXISTS (SELECT * FROM task AS task2 WHERE task2.id = ANY (task.preceding_task_ids) AND task2.status_id != 6 /*Finished*/ );

                    -- Get the list of tasks that depended on tasks that HAVE been finished
                    SELECT array_agg(task.id) INTO runnable_task_ids FROM task
                    WHERE task.job_id = _job_id
                      AND task.status_id = 5 --Paused
                      AND NOT EXISTS (SELECT * FROM task AS task2 WHERE task2.id = ANY (task.preceding_task_ids) AND task2.status_id != 6 /*Finished*/ );

                    -- Update the tasks that CANNOT be started right now
                    UPDATE task
                    SET status_id = 3, --NeedsInput
                    status_timestamp = now()
                    WHERE id = ANY (unrunnable_task_ids);

                    -- Update the tasks that CAN be started right now
                    UPDATE task
                    SET status_id = CASE WHEN EXISTS (SELECT * FROM step WHERE step.task_id = task.id AND step.status_id = 6) THEN 4 --Running
                            ELSE 1 --Submitted
                            END,
                    status_timestamp = now()
                    WHERE id = ANY (runnable_task_ids);

                    processor_id := (SELECT job.processor_id FROM job WHERE id = _job_id);

                    IF runnable_task_ids IS NOT NULL THEN
                        -- Add events for all the runnable tasks
                        FOREACH runnable_task_id IN ARRAY runnable_task_ids
                        LOOP
                                INSERT INTO event(
                                type_id,
                                data,
                                submitted_timestamp)
                                VALUES (
                                1, -- TaskRunnable
                                ('{"job_id":' || _job_id || ', "processor_id":' || processor_id || ', "task_id":' || runnable_task_id || '}') :: json,
                                now());
                        END LOOP;
                    END IF;

                    UPDATE job
                    SET status_id = CASE WHEN EXISTS (SELECT * FROM task WHERE task.job_id = job.id AND status_id IN (4,6)) THEN 4 --Running
                            ELSE 1 --Submitted
                            END,
                    status_timestamp = now()
                    WHERE id = _job_id
                    AND status_id = 5; --Paused

                END;
                $$
                LANGUAGE plpgsql VOLATILE;

                $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement = 'DROP FUNCTION IF EXISTS sp_mark_job_paused(int)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_mark_job_paused(
                IN _job_id int
                ) RETURNS void AS $$

                BEGIN

                    IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
                        RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
                    END IF;

                    UPDATE step
                    SET status_id = 5, --Paused
                    status_timestamp = now()
                    FROM task
                    WHERE task.id = step.task_id AND task.job_id = _job_id
                    AND step.status_id NOT IN (5, 6, 7, 8); -- Finished, cancelled or failed steps can't be paused

                    UPDATE task
                    SET status_id = 5, --Paused
                    status_timestamp = now()
                    WHERE job_id = _job_id
                    AND status_id NOT IN (5, 6, 7, 8); -- Finished, cancelled or failed tasks can't be paused

                    UPDATE job
                    SET status_id = 5, --Paused
                    status_timestamp = now()
                    WHERE id = _job_id
                    AND status_id NOT IN (5, 6, 7, 8); -- Finished, cancelled or failed jobs can't be paused

                END;
                $$ LANGUAGE plpgsql;

                $str$;
            raise notice '%', _statement;
            execute _statement;
            

            _statement = 'DROP FUNCTION IF EXISTS sp_mark_job_cancelled(int)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_mark_job_cancelled(
                IN _job_id int
                ) RETURNS void AS $$
                BEGIN

                    IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
                        RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
                    END IF;

                    UPDATE step
                    SET status_id = 7, --Cancelled
                    status_timestamp = now()
                    FROM task
                    WHERE task.id = step.task_id AND task.job_id = _job_id
                    AND step.status_id NOT IN (6, 8) -- Finished or failed steps can't be cancelled
                    AND step.status_id != 7; -- Prevent resetting the status on serialization error retries.

                    UPDATE task
                    SET status_id = 7, --Cancelled
                    status_timestamp = now()
                    WHERE job_id = _job_id
                    AND status_id NOT IN (6, 8) -- Finished or failed tasks can't be cancelled
                    AND status_id != 7; -- Prevent resetting the status on serialization error retries.

                    UPDATE job
                    SET status_id = 7, --Cancelled
                    status_timestamp = now()
                    WHERE id = _job_id
                    AND status_id NOT IN (6, 7, 8); -- Finished or failed jobs can't be cancelled
                END;
                $$ LANGUAGE plpgsql;
                $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement = 'DROP FUNCTION IF EXISTS sp_dashboard_remove_scheduled_task(smallint)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_dashboard_remove_scheduled_task(_schedule_id smallint)
                  RETURNS void AS
                $BODY$
                BEGIN 

                DELETE FROM scheduled_task WHERE id = _schedule_id;

                DELETE FROM scheduled_task_status WHERE task_id = _schedule_id;

                END;
                $BODY$
                  LANGUAGE plpgsql;
                $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement = 'DROP FUNCTION IF EXISTS sp_changepassword(smallint, character varying, character varying)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION "sp_changepassword"("userId" smallint, "oldPassword" character varying, "newPassword" character varying)
                  RETURNS void AS
                $BODY$UPDATE public.user
                         SET password = crypt($3, gen_salt('md5'))
                         WHERE id = $1 AND password = crypt($2, password)$BODY$
                  LANGUAGE sql VOLATILE;
                $str$;
            raise notice '%', _statement;
            execute _statement;
            
            _statement := $str$
                CREATE TABLE if not exists datasource
                (
                  id smallserial NOT NULL,
                  satellite_id smallint NOT NULL,
                  name character varying(50) NOT NULL,
                  scope smallint NOT NULL DEFAULT 3,
                  username character varying(100),
                  passwrd character varying(100),
                  fetch_mode smallint NOT NULL DEFAULT 1,
                  max_retries integer NOT NULL DEFAULT 3,
                  retry_interval_minutes integer NOT NULL DEFAULT 3600,
                  download_path character varying(255),
                  specific_params json,
                  max_connections integer NOT NULL DEFAULT 1,
                  local_root character varying(255),
                  enabled boolean NOT NULL DEFAULT false,
                  CONSTRAINT pk_datasource PRIMARY KEY (id),
                  CONSTRAINT datasource_satellite_id_fkey FOREIGN KEY (satellite_id)
                      REFERENCES satellite (id) MATCH SIMPLE
                      ON UPDATE NO ACTION ON DELETE NO ACTION
                )
                WITH (
                  OIDS=FALSE
                );
                ALTER TABLE datasource
                  OWNER TO admin;
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            if not exists (select * from downloader_status where id = 6) then
                _statement := $str$
                INSERT INTO downloader_status(id,status_description) VALUES (6, 'processing_failed');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from downloader_status where id = 7) then
                _statement := $str$
                INSERT INTO downloader_status(id,status_description) VALUES (7, 'processing');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            if not exists (select * from satellite where id = 3) then
                _statement := $str$
                INSERT INTO satellite(id,satellite_name) VALUES (3, 'sentinel1');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;


            _statement = 'DROP FUNCTION IF EXISTS sp_get_dashboard_products(smallint,smallint)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_dashboard_products(_site_id smallint DEFAULT NULL::smallint, _product_type_id integer[] DEFAULT NULL::integer[], _season_id smallint DEFAULT NULL::smallint, _satellit_id integer[] DEFAULT NULL::integer[], _since_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone, _until_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone, _tiles character varying[] DEFAULT NULL::character varying[])
                  RETURNS SETOF json AS
                $BODY$
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
                                data(id, satellite_id, product, product_type_id, product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
                                SELECT
                                P.id,
                                P.satellite_id,
                                P.name,
                                PT.id,
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
                                AND EXISTS (
                                    SELECT * FROM season WHERE season.site_id =P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date 
                            $sql$;
                            IF $3 IS NOT NULL THEN
                            q := q || $sql$
                                AND season.id=$3			       
                                $sql$;
                            END IF;
                                
                            q := q || $sql$
                            )
                            $sql$;
                            
                            IF $1 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.site_id = $1		    
                                
                            $sql$;
                            END IF;
                            
                            IF $2 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.product_type_id= ANY($2)

                                $sql$;
                            END IF;

                        --IF $4 IS NOT NULL THEN
                        --q := q || $sql$
                            --AND  P.satellite_id = ANY($4)			  
                            --$sql$;
                        --END IF;
                            
                        IF $5 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp >= to_timestamp(cast($5 as TEXT),'YYYY-MM-DD HH24:MI:SS') 
                            $sql$;
                        END IF;

                        IF $6 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp <= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day' 
                            $sql$;
                        END IF;

                        IF $7 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.tiles <@$7 AND P.tiles!='{}'
                            $sql$;
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
                            USING _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles;
                        END
                        $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION sp_get_dashboard_products(smallint, integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[]) OWNER TO "admin";
                $str$;
            raise notice '%', _statement;
            execute _statement;
            
            
            _statement = 'DROP FUNCTION IF EXISTS sp_get_products_sites(smallint)';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_products_sites(IN _site_id smallint DEFAULT NULL::smallint)
                  RETURNS TABLE(id smallint, "name" character varying, short_name character varying, enabled boolean) AS
                $BODY$
                                BEGIN
                                RETURN QUERY
                                        SELECT DISTINCT (S.id),
                                            S.name,
                                            S.short_name,
                                            S.enabled
                                        FROM site S
                                        JOIN product P ON P.site_id = S.id
                                        WHERE _site_id IS NULL OR S.id = _site_id
                                        ORDER BY S.name;
                                END
                                $BODY$
                  LANGUAGE plpgsql VOLATILE;
                $str$;
            raise notice '%', _statement;
            execute _statement;
            
            _statement = 'DROP FUNCTION IF EXISTS sp_get_product_types()';
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_product_types()
                  RETURNS TABLE(id smallint, description character varying, "name" character varying) AS
                $BODY$
                BEGIN
                    RETURN QUERY
                        SELECT product_type.id,
                               product_type.description,
                               product_type.name
                        FROM product_type
                        ORDER BY product_type.id;
                END
                $BODY$
                  LANGUAGE plpgsql STABLE;
                $str$;
            raise notice '%', _statement;
            execute _statement;
            
            
            _statement := $str$
                CREATE TABLE downloader_count
                (
                    site_id smallint NOT NULL,
                    satellite_id smallint NOT NULL,
                    product_count integer NOT NULL,
                    start_date date NOT NULL,
                    end_date date NOT NULL,
                    last_updated timestamp with time zone DEFAULT now(),
                    CONSTRAINT pk_donwloader_count PRIMARY KEY (site_id, satellite_id, start_date, end_date),
                    CONSTRAINT fk_downloader_count_satellite FOREIGN KEY (satellite_id)
                        REFERENCES satellite (id) MATCH SIMPLE
                        ON UPDATE NO ACTION
                        ON DELETE NO ACTION,
                    CONSTRAINT fk_downloader_count_site FOREIGN KEY (site_id)
                        REFERENCES site (id) MATCH SIMPLE
                        ON UPDATE NO ACTION
                        ON DELETE NO ACTION
                )
                WITH (
                    OIDS = FALSE
                )
                TABLESPACE pg_default;

                ALTER TABLE downloader_count
                    OWNER to admin;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE INDEX fki_fk_downloader_count_satellite
                    ON downloader_count USING btree
                    (satellite_id)
                    TABLESPACE pg_default;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE INDEX fki_fk_downloader_count_site
                    ON downloader_count USING btree
                    (site_id)
                    TABLESPACE pg_default;
            $str$;
            raise notice '%', _statement;
            execute _statement;
           
            _statement := 'update meta set version = ''1.8'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
