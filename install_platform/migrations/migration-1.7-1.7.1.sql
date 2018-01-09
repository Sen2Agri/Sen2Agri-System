begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.7') then
            raise notice 'upgrading from 1.7 to 1.7.1';

            if not exists (select * from config_metadata where key = 'processor.l3b.lai.use_belcam_version') then
                _statement := $str$
                INSERT INTO config_metadata VALUES ('processor.l3b.lai.use_belcam_version', 'L3B LAI processor will use Belcam implementation', 'int', false, 4);
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

            _statement := 'update meta set version = ''1.7.1'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
