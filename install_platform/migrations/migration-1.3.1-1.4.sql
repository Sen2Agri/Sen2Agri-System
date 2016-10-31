begin transaction;

do $migration$
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.3.1') then
            raise notice 'upgrading from 1.3.1 to 1.4';

            raise notice 'applying d60950703ca3989963cae1bc2bbeea10a569225a';
            raise notice 'update config set value = ''/usr/share/sen2agri/crop-mask.map'' where key = ''processor.l4a.lut_path'';';
            update config set value = '/usr/share/sen2agri/crop-mask.map' where key = 'processor.l4a.lut_path';

            raise notice 'applying f44ceba163217ae7612d6167ef549cc79fcd7ad8';
            raise notice 'update config set value = ''1'' where key = ''processor.l3a.generate_20m_s2_resolution'';';
            update config set value = '1' where key = 'processor.l3a.generate_20m_s2_resolution';

            raise notice 'update meta set version = ''1.4'';';
            update meta set version = '1.4';
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
