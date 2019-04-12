begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations from sen2agri to sen4cap';

    if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='downloader_history' and column_name='footprint') then
        _statement := $str$
        ALTER TABLE downloader_history ADD COLUMN footprint geography null;
        $str$;
        raise notice '%', _statement;
        execute _statement;
    end if;
    
    
    if not exists (select * from config_metadata where key = 's1.preprocessing.path') then
        _statement := $str$
        INSERT INTO config_metadata VALUES ('s1.preprocessing.path', 'The path where the S1 L2 products will be created', 'string', false, 15);
        $str$;
        raise notice '%', _statement;
        execute _statement;
    end if;
    if not exists (select * from config where key = 's1.preprocessing.path' and site_id is null) then
        _statement := $str$
        INSERT INTO config(key, site_id, value, last_updated) VALUES ('s1.preprocessing.path', NULL, '/mnt/archive/{site}/l2a-s1', '2017-10-24 14:56:57.501918+02');
        $str$;
        raise notice '%', _statement;
        execute _statement;
    end if;

    if not exists (select * from config_metadata where key = 's1.preprocessing.enabled') then
        _statement := $str$
        INSERT INTO config_metadata VALUES ('s1.preprocessing.enabled', 'S1 preprocessing is enabled', 'bool', false, 15);
        $str$;
        raise notice '%', _statement;
        execute _statement;
    end if;
    if not exists (select * from config where key = 's1.preprocessing.enabled' and site_id is null) then
        _statement := $str$
        INSERT INTO config(key, site_id, value, last_updated) VALUES ('s1.preprocessing.enabled', NULL, 'false', '2017-10-24 14:56:57.501918+02');
        $str$;
        raise notice '%', _statement;
        execute _statement;
    end if;
    
    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.8.1') then
            raise notice 'upgrading from 1.8.1 to 1.8.2';
           
            _statement := 'update meta set version = ''1.8.2'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
