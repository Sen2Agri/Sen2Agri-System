<<<<<<< HEAD
begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('1.0.0', '2.0.0-RC1', '1.0.1')) then
            raise notice 'upgrading from 1.0.0 to 1.0.1';
            
-- config_category  updates
            _statement := $str$
                DELETE FROM config_category WHERE id = 20 and name = 'L4B Grassland Mowing';
            $str$;
            raise notice '%', _statement;
            execute _statement;
            if not exists (select * from config_category where id = 20 and name = 'L4C Agricultural Practices') then
               _statement := $str$
                   INSERT INTO config_category VALUES (20, 'L4C Agricultural Practices', 16, true);
               $str$;
               raise notice '%', _statement;
               execute _statement;
            end if;

-- Processors updates

            _statement := $str$
                DELETE FROM processor WHERE name like 'L2 SAR%';
            $str$;
            raise notice '%', _statement;
            execute _statement;

            if not exists (select * from processor where short_name = 'l2-s1') then
                _statement := $str$
                    INSERT INTO processor(id, name, short_name, label) VALUES (7, 'L2-S1 Pre-Processor', 'l2-s1', 'L2 S1 &mdash; SAR Pre-Processor');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;           
            
-- Script paths updates
            _statement := $str$
                DELETE FROM config WHERE key = 'executor.module.path.lpis_import';
                DELETE FROM config WHERE key = 'executor.module.path.l4b_cfg_import';
                DELETE FROM config WHERE key = 'executor.module.path.l4c_cfg_import';
                DELETE FROM config WHERE key = 'executor.module.path.l4c_practices_import';
                DELETE FROM config WHERE key = 'executor.module.path.l4c_practices_export';
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                INSERT INTO config(key, value) VALUES ('executor.module.path.lpis_import', '/usr/bin/data-preparation.py');
                INSERT INTO config(key, value) VALUES ('executor.module.path.l4b_cfg_import', '/usr/bin/s4c_l4b_import_config.py');
                INSERT INTO config(key, value) VALUES ('executor.module.path.l4c_cfg_import', '/usr/bin/s4c_l4c_import_config.py');
                INSERT INTO config(key, value) VALUES ('executor.module.path.l4c_practices_import', '/usr/bin/s4c_l4c_import_practice.py');
                INSERT INTO config(key, value) VALUES ('executor.module.path.l4c_practices_export', '/usr/bin/s4c_l4c_export_all_practices.py');
            $str$;
            raise notice '%', _statement;
            execute _statement;

            
-- Agricultural practices updates
            _statement := $str$
                DELETE FROM config WHERE key = 'processor.s4c_l4c.ndvi_data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.amp_data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.cohe_data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.config_path';                
                
                -- deletions for updating the same version
                DELETE FROM config WHERE key = 'executor.processor.s4c_l4c.keep_job_folders';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.cfg_upload_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.ts_input_tables_upload_root_dir';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4c.default_config_path';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.cfg_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.ts_input_tables_dir';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4c.practices';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.country';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.tsa_min_acqs_no';
                DELETE FROM config WHERE key = 'executor.module.path.ogr2ogr';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.use_prev_prd';
                -- END of deletions for updating the same version
            $str$;
            raise notice '%', _statement;
            execute _statement;
                
            _statement := $str$                
                INSERT INTO config(key, value) VALUES ('executor.processor.s4c_l4c.keep_job_folders', '0');
                INSERT INTO config(key, value) VALUES ('executor.module.path.ogr2ogr', '/usr/local/bin/ogr2ogr');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.cfg_upload_dir', '/mnt/archive/upload/agric_practices_files/{site}/config');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.ts_input_tables_upload_root_dir', '/mnt/archive/upload/agric_practices_files/{site}/ts_input_tables');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.default_config_path', '/usr/share/sen2agri/S4C_L4C_Configurations/S4C_L4C_Default_Config.cfg');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.cfg_dir', '/mnt/archive/agric_practices_files/{site}/{year}/config/');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.data_extr_dir', '/mnt/archive/agric_practices_files/{site}/{year}/data_extraction/{product_type}');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.ts_input_tables_dir', '/mnt/archive/agric_practices_files/{site}/{year}/ts_input_tables/{practice}');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.practices', 'NA');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.country', 'CNTRY');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.tsa_min_acqs_no', '15');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.use_prev_prd', '1');
                
            $str$;
            raise notice '%', _statement;
            execute _statement;
-- LPIS updates
            _statement := $str$
                DELETE from config WHERE key = 'processor.lpis.upload_path';
                DELETE from config WHERE key = 'processor.lpis.lut_upload_path';
                DELETE from config WHERE key = 'processor.lpis.path';
                
                INSERT INTO config(key, value) VALUES ('processor.lpis.upload_path', '/mnt/archive/upload/lpis/{site}');
                INSERT INTO config(key, value) VALUES ('processor.lpis.lut_upload_path', '/mnt/archive/upload/LUT/{site}');
                INSERT INTO config(key, value) VALUES ('processor.lpis.path', '/mnt/archive/lpis/{site}/{year}');
            $str$;
            raise notice '%', _statement;
            execute _statement;
    
-- Grassland mowing updates
            _statement := $str$
                 DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-mowing-s1';
                 DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-mowing-s2';
                 DELETE FROM config WHERE key = 'processor.s4c_l4b.config_path';
                 
                 -- deletions for updating the same version
                DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-gen-input-shp';
                DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-mowing';
                DELETE FROM config WHERE key = 'executor.processor.s4c_l4b.keep_job_folders';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4b.cfg_upload_dir';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4b.default_config_path';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.gen_input_shp_path';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.s1_py_script';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.s2_py_script';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.sub_steps';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.input_product_types';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.cfg_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.gen_shp_py_script';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4b.start_date';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.end_date';
                
                DELETE FROM config_metadata WHERE key = 'processor.s4c_l4b.start_date';
                DELETE FROM config_metadata WHERE key = 'processor.s4c_l4b.end_date';
                 -- END of deletions for updating the same version
             $str$;
             raise notice '%', _statement;
             execute _statement;
 

            _statement := $str$
                INSERT INTO config(key, value) VALUES ('executor.module.path.s4c-grassland-gen-input-shp', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/generate_grassland_mowing_input_shp.sh');
                INSERT INTO config(key, value) VALUES ('executor.module.path.s4c-grassland-mowing', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/grassland_mowing.sh');
                INSERT INTO config(key, value) VALUES ('executor.processor.s4c_l4b.keep_job_folders', 0);
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.cfg_upload_dir', '/mnt/archive/upload/grassland_mowing_cfg/{site}');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.default_config_path', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_ini/S4C_L4B_Default_Config.cfg');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.gen_input_shp_path', '/mnt/archive/grassland_mowing_files/{site}/{year}/InputShp/SEN4CAP_L4B_GeneratedInputShp.shp');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.s1_py_script', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_s1/S1_main.py');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.s2_py_script', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_s2/S2_main.py');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.sub_steps', 'S1_S2, S1, S2');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.input_product_types', 'S1_S2');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.cfg_dir', '/mnt/archive/grassland_mowing_files/{site}/{year}/config/');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.gen_shp_py_script', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/generate_grassland_mowing_input_shp.py');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.start_date', '');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.end_date', '');
                
                INSERT INTO config_metadata VALUES ('processor.s4c_l4b.start_date', 'Start date for the mowing detection', 'string', FALSE, 19, TRUE, 'Start date for the mowing detection');
                INSERT INTO config_metadata VALUES ('processor.s4c_l4b.end_date', 'End date for the mowing detection', 'string', FALSE, 19, TRUE, 'End date for the mowing detection');

            $str$;
            raise notice '%', _statement;
            execute _statement;

-- Function updates            
            _statement := $str$
                create or replace function sp_insert_default_scheduled_tasks(
                    _season_id season.id%type,
                    _processor_id processor.id%type default null
                )
                returns void as
                $$
                declare _site_id site.id%type;
                declare _site_name site.short_name%type;
                declare _processor_name processor.short_name%type;
                declare _season_name season.name%type;
                declare _start_date season.start_date%type;
                declare _mid_date season.start_date%type;
                begin
                    select site.short_name
                    into _site_name
                    from season
                    inner join site on site.id = season.site_id
                    where season.id = _season_id;

                    select processor.short_name
                    into _processor_name
                    from processor
                    where id = _processor_id;

                    if not found then
                        raise exception 'Invalid season id %', _season_id;
                    end if;

                    select site_id,
                           name,
                           start_date,
                           mid_date
                    into _site_id,
                         _season_name,
                         _start_date,
                         _mid_date
                    from season
                    where id = _season_id;

                    if _processor_id is null or (_processor_id = 2 and _processor_name = 'l3a') then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L3A' :: character varying,
                                    2,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast((select date_trunc('month', _start_date) + interval '1 month' - interval '1 day') as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or (_processor_id = 3 and _processor_name = 'l3b_lai')  then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L3B' :: character varying,
                                    3,
                                    _site_id :: int,
                                    _season_id :: int,
                                    1::smallint,
                                    1::smallint,
                                    0::smallint,
                                    cast((_start_date + 1) as character varying),
                                    60,
                                    1 :: smallint,
                                    '{"general_params":{"product_type":"L3B"}}' :: json);
                    end if;

                    if _processor_id is null or (_processor_id = 5 and _processor_name = 'l4a') then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L4A' :: character varying,
                                    5,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast(_mid_date as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or (_processor_id = 6 and _processor_name = 'l4b') then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L4B' :: character varying,
                                    6,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast(_mid_date as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;
                    
                    if _processor_id is null or _processor_name = 's4c_l4a' then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_S4C_L4A' :: character varying,
                                    4,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast(_mid_date as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or _processor_name = 's4c_l4b' then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_S4C_L4B' :: character varying,
                                    5,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast((_start_date + 31) as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or _processor_name = 's4c_l4c' then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_S4C_L4C' :: character varying,
                                    6,
                                    _site_id :: int,
                                    _season_id :: int,
                                    1::smallint,
                                    7::smallint,
                                    0::smallint,
                                    cast((_start_date + 7) as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;


                    if _processor_id is not null and (_processor_id not in (2, 3, 5, 6) and _processor_name not in ('s4c_l4a', 's4c_l4b', 's4c_l4c')) then
                        raise exception 'No default jobs defined for processor id %', _processor_id;
                    end if;

                end;
                $$
                    language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := 'update meta set version = ''1.0.1'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('1.0.0', '2.0.0-RC1', '1.0.1')) then
            raise notice 'upgrading from 1.0.0 to 1.0.1';
            
-- Agricultural practices updates
            _statement := $str$
                DELETE FROM config WHERE key = 'processor.s4c_l4c.ndvi_data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.amp_data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.cohe_data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.config_path';                
                
                -- deletions for updating the same version
                DELETE FROM config WHERE key = 'executor.processor.s4c_l4c.keep_job_folders';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.cfg_upload_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.ts_input_tables_upload_root_dir';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4c.default_config_path';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.cfg_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.data_extr_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.ts_input_tables_dir';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4c.practices';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.country';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.tsa_min_acqs_no';
                DELETE FROM config WHERE key = 'executor.module.path.ogr2ogr';
                DELETE FROM config WHERE key = 'processor.s4c_l4c.use_prev_prd';
                -- END of deletions for updating the same version
                
                INSERT INTO config(key, value) VALUES ('executor.processor.s4c_l4c.keep_job_folders', '0');
                INSERT INTO config(key, value) VALUES ('executor.module.path.ogr2ogr', '/usr/local/bin/ogr2ogr');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.cfg_upload_dir', '/mnt/archive/agric_practices_files/upload/{site}');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.ts_input_tables_upload_root_dir', '/mnt/archive/agric_practices_files/upload/{site}/ts_input_tables');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.default_config_path', '/usr/share/sen2agri/S4C_L4C_Configurations/S4C_L4C_Default_Config.cfg');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.cfg_dir', '/mnt/archive/agric_practices_files/{site}/{year}/config/');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.data_extr_dir', '/mnt/archive/agric_practices_files/{site}/{year}/data_extraction/{product_type}');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.ts_input_tables_dir', '/mnt/archive/agric_practices_files/{site}/{year}/ts_input_tables/{practice}');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.practices', 'NA');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.country', 'CNTRY');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.tsa_min_acqs_no', '15');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.use_prev_prd', '1');
                
            $str$;
            raise notice '%', _statement;
            execute _statement;
-- LPIS updates
            _statement := $str$
                DELETE from config WHERE key = 'processor.lpis.upload_path';
                DELETE from config WHERE key = 'processor.lpis.path';
                
                INSERT INTO config(key, value) VALUES ('processor.lpis.upload_path', '/mnt/archive/lpis/upload/{site}');
                INSERT INTO config(key, value) VALUES ('processor.lpis.path', '/mnt/archive/lpis/{site}/{year}');
            $str$;
            raise notice '%', _statement;
            execute _statement;
    



-- Grassland mowing updates
            _statement := $str$
                 DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-mowing-s1';
                 DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-mowing-s2';
                 DELETE FROM config WHERE key = 'processor.s4c_l4b.config_path';
                 
                 -- deletions for updating the same version
                DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-gen-input-shp';
                DELETE FROM config WHERE key = 'executor.module.path.s4c-grassland-mowing';
                DELETE FROM config WHERE key = 'executor.processor.s4c_l4b.keep_job_folders';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4b.cfg_upload_dir';
                
                DELETE FROM config WHERE key = 'processor.s4c_l4b.default_config_path';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.gen_input_shp_path';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.s1_py_script';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.s2_py_script';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.sub_steps';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.input_product_types';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.cfg_dir';
                DELETE FROM config WHERE key = 'processor.s4c_l4b.gen_shp_py_script';
                 -- END of deletions for updating the same version
             $str$;
             raise notice '%', _statement;
             execute _statement;
 

            _statement := $str$
                INSERT INTO config(key, value) VALUES ('executor.module.path.s4c-grassland-gen-input-shp', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/generate_grassland_mowing_input_shp.sh');
                INSERT INTO config(key, value) VALUES ('executor.module.path.s4c-grassland-mowing', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/grassland_mowing.sh');
                INSERT INTO config(key, value) VALUES ('executor.processor.s4c_l4b.keep_job_folders', 0);
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.cfg_upload_dir', '/mnt/archive/grassland_mowing_files/upload/{site}');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.default_config_path', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_ini/S4C_L4B_Default_Config.ini');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.gen_input_shp_path', '/mnt/archive/grassland_mowing_files/{site}/{year}/InputShp/SEN4CAP_L4B_GeneratedInputShp.shp');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.s1_py_script', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_s1/S1_main.py');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.s2_py_script', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_s2/S2_main.py');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.sub_steps', 'S1_S2, S1, S2');
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.input_product_types', 'S1_S2');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.cfg_dir', '/mnt/archive/grassland_mowing_files/{site}/{year}/config/');
                
                INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.gen_shp_py_script', '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/generate_grassland_mowing_input_shp.py');
                 
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            
            _statement := 'update meta set version = ''1.0.1'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
