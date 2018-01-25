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
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.use_belcam_version', 'L3B LAI processor will use INRA algorithm implementation', 'int', false, 4);
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'processor.l3b.lai.use_belcam_version' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.use_belcam_version', NULL, '0', '2017-10-24 14:56:57.501918+02');
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
            
            

            _statement := 'update meta set version = ''1.7.1'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
