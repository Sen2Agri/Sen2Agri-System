begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='downloader_history' and column_name='footprint') then
        _statement := $str$
        ALTER TABLE downloader_history ADD COLUMN footprint geography null;
        $str$;
        raise notice '%', _statement;
        execute _statement;
    end if;
    
    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.8.2') then
            raise notice 'upgrading from 1.8.2 to 1.8.3';
           
            _statement := 'update meta set version = ''1.8.3'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
