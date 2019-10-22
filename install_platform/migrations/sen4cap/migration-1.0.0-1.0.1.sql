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
