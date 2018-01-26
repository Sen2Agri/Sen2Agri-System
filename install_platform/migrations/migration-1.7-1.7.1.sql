begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.7') then
            raise notice 'upgrading from 1.7 to 1.7.1';

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

            _statement := $str$
            drop function if exists sp_insert_product(smallint, smallint, integer, smallint, integer, character varying, timestamp with time zone, character varying, character varying, geography, json);
            $str$;
            raise notice '%', _statement;
            execute _statement;


            if not exists (select * from config_metadata where key = 'downloader.stopped') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.stopped', 'Downloader is stopped', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.stopped' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.stopped', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.S1.stopped') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.S1.stopped', 'S1 downloader is stopped', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.S1.stopped' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.S1.stopped', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.S2.stopped') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.S2.stopped', 'S2 downloader is stopped', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.S2.stopped' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.S2.stopped', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.L8.stopped') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.L8.stopped', 'L8 downloader is stopped', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.L8.stopped' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.L8.stopped', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'S1.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('S1.enabled', 'S1 is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'S1.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('S1.enabled', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'S2.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('S2.enabled', 'S2 is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'S2.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('S2.enabled', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'L8.enabled') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('L8.enabled', 'L8 is enabled', 'bool', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'L8.enabled' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('L8.enabled', NULL, '0', '2017-10-24 14:56:57.501918+02');
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
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.lookup.enabled', NULL, '0', '2017-10-24 14:56:57.501918+02');
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
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.retry.enabled', NULL, '0', '2017-10-24 14:56:57.501918+02');
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
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.object.storage.move.enabled', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s2.datasource.query') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s2.datasource.query', 'S2 datasource query location', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s2.datasource.query' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.datasource.query', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s2.datasource.query.user') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s2.datasource.query.user', 'S2 datasource query location user', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s2.datasource.query.user' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.datasource.query.user', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s2.datasource.query.password') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s2.datasource.query.password', 'S2 datasource query location password', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s2.datasource.query.password' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.datasource.query.password', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s2.datasource.download') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s2.datasource.download', 'S2 datasource download location', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s2.datasource.download' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.datasource.download', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s2.datasource.download.user') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s2.datasource.download.user', 'S2 datasource download location user', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s2.datasource.download.user' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.datasource.download.user', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.s2.datasource.download.password') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.s2.datasource.download.password', 'S2 datasource download location password', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.s2.datasource.download.password' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.datasource.download.password', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.l8.datasource.query') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.l8.datasource.query', 'L8 datasource query location', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.l8.datasource.query' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.datasource.query', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.l8.datasource.query.user') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.l8.datasource.query.user', 'L8 datasource query location user', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.l8.datasource.query.user' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.datasource.query.user', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.l8.datasource.query.password') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.l8.datasource.query.password', 'L8 datasource query location password', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.l8.datasource.query.password' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.datasource.query.password', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.l8.datasource.download') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.l8.datasource.download', 'L8 datasource download location', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.l8.datasource.download' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.datasource.download', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.l8.datasource.download.user') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.l8.datasource.download.user', 'L8 datasource download location user', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.l8.datasource.download.user' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.datasource.download.user', NULL, '0', '2017-10-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config_metadata where key = 'downloader.l8.datasource.download.password') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('downloader.l8.datasource.download.password', 'L8 datasource download location password', 'string', false, 15);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'downloader.l8.datasource.download.password' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.datasource.download.password', NULL, '0', '2017-10-24 14:56:57.501918+02');
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

            _statement := 'update meta set version = ''1.7.1'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
