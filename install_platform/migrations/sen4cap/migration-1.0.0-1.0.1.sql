begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('1.0.0', '2.0.0-RC1', '1.0.1', '1.1')) then
            _statement := $str$                
                CREATE OR REPLACE FUNCTION public.check_season()
                        RETURNS TRIGGER AS
                    $BODY$
                    BEGIN
                        IF NOT EXISTS (SELECT id FROM public.season WHERE id != NEW.id AND site_id = NEW.site_id AND enabled = true AND start_date <= NEW.start_date AND end_date >= NEW.end_date) THEN
                            RETURN NEW;
                        ELSE
                            RAISE EXCEPTION 'Nested seasons are not allowed';
                        END IF;
                    END;
                    $BODY$
                    LANGUAGE plpgsql VOLATILE;
            $str$;
            raise notice '%', _statement;
            execute _statement;    

            if not exists (select tgname from pg_trigger where not tgisinternal and tgrelid = 'season'::regclass and tgname = 'check_season_dates') then
                _statement := $str$
                    CREATE TRIGGER check_season_dates 
                        BEFORE INSERT OR UPDATE ON public.season
                        FOR EACH ROW EXECUTE PROCEDURE public.check_season();
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config where key = 'processor.s4c_l4a.mode') then
                _statement := $str$
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.mode', NULL, 'both', '2019-02-19 11:09:58.820032+02');
                    INSERT INTO config_metadata VALUES ('processor.s4c_l4a.mode', 'Mode', 'string', FALSE, 5, TRUE, 'Mode (both, s1-only, s2-only)', '{"min":"","step":"","max":""}');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
        
            if not exists (select * from config where key = 'scheduled.reports.enabled') then
                _statement := $str$
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.reports.enabled', NULL, 'true', '2020-05-04 14:56:57.501918+02');
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.reports.interval', NULL, '24', '2020-05-04 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'scheduled.reports.interval') then
                _statement := $str$
                    UPDATE config SET value = 'true' WHERE key = 'scheduled.reports.enabled';
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.reports.interval', NULL, '24', '2020-05-04 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            

            -- new tables creation, if not exist 
            _statement := $str$
                create table if not exists agricultural_practice(
                    id int not null primary key,
                    name text not null
                );
                
                create table if not exists product_provenance(
                    product_id int not null,
                    parent_product_id int not null,
                    parent_product_date timestamp with time zone not null,
                    constraint product_provenance_pkey primary key (product_id, parent_product_id)
                );
                
                create or replace function sp_insert_product_provenance(
                    _product_id int,
                    _parent_product_id int,
                    _parent_product_date int
                )
                returns void
                as $$
                begin
                    insert into product_provenance(product_id, parent_product_id, parent_product_date)
                    values (_product_id, _parent_product_id, _parent_product_date);
                end;
                $$
                language plpgsql volatile;
                
                create or replace function sp_is_product_stale(
                    _product_id int,
                    _parent_products int[]
                )
                returns boolean
                as $$
                begin
                    return exists (
                        select pid
                        from unnest(_parent_products) as pp(pid)
                        where not exists (
                            select *
                            from product_provenance
                            where (product_id, parent_product_id) = (_product_id, pid)
                        )
                    ) or exists (
                        select *
                        from product_provenance
                        where exists (
                            select *
                            from product
                            where product.id = product_provenance.parent_product_id
                              and product.created_timestamp >= product_provenance.parent_product_date
                        )
                    );
                end;
                $$
                language plpgsql stable;     

                INSERT INTO agricultural_practice(id, name) SELECT 1, 'NA' WHERE NOT EXISTS (SELECT 1 FROM agricultural_practice WHERE id=1);
                INSERT INTO agricultural_practice(id, name) SELECT 2, 'CatchCrop' WHERE NOT EXISTS (SELECT 1 FROM agricultural_practice WHERE id=2);
                INSERT INTO agricultural_practice(id, name) SELECT 3, 'NFC' WHERE NOT EXISTS (SELECT 1 FROM agricultural_practice WHERE id=3);
                INSERT INTO agricultural_practice(id, name) SELECT 4, 'Fallow' WHERE NOT EXISTS (SELECT 1 FROM agricultural_practice WHERE id=4);

                UPDATE config SET VALUE = 1333 WHERE key = 'processor.s4c_l4a.pa-train-l';

                create table if not exists product_details_l4a(
                    product_id int not null references product(id) on delete cascade,
                    "NewID" int not null,
                    "CT_decl" int,
                    "CT_pred_1" int,
                    "CT_conf_1" real,
                    "CT_pred_2" int,
                    "CT_conf_2" real,
                    constraint product_details_l4a_pkey primary key(product_id, "NewID")
                );
            
                create table if not exists product_details_l4c(
                    product_id int not null references product(id) on delete cascade,
                    "NewID" int not null,
                    practice_id int not null references agricultural_practice(id),
                    orig_id text not null,
                    country text not null,
                    year int not null,
                    main_crop text not null,
                    veg_start text not null,
                    h_start text not null,
                    h_end text not null,
                    practice text not null,
                    p_type text not null,
                    p_start text not null,
                    p_end text not null,
                    l_week text not null,
                    m1 text not null,
                    m2 text not null,
                    m3 text not null,
                    m4 text not null,
                    m5 text not null,
                    h_week text not null,
                    h_w_start text not null,
                    h_w_end text not null,
                    h_w_s1 text not null,
                    m6 text not null,
                    m7 text not null,
                    m8 text not null,
                    m9 text not null,
                    m10 text not null,
                    c_index text not null,
                    s1_pix text not null,
                    s1_gaps text not null,
                    h_s1_gaps text not null,
                    p_s1_gaps text not null,
                    h_w_s1_gaps text,
                    h_quality text,
                    c_quality text,    
                    constraint product_details_l4c_pkey primary key(product_id, "NewID")
                );
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            -- product_details_l4c table updates
            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='product_details_l4c' and column_name='h_w_s1_gaps') then
                _statement := $str$
                    -- these are all added in the same time so if one exists, the others also exist
                    ALTER TABLE product_details_l4c ADD COLUMN h_w_s1_gaps text, ADD COLUMN h_quality text, ADD COLUMN c_quality text;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            
            _statement := $str$
                DROP FUNCTION IF EXISTS sp_get_dashboard_products_nodes(integer[], integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[], boolean);
                
                CREATE OR REPLACE FUNCTION sp_get_dashboard_products_nodes(
                    _user_name character varying,
                    _site_id integer[] DEFAULT NULL::integer[],
                    _product_type_id integer[] DEFAULT NULL::integer[],
                    _season_id smallint DEFAULT NULL::smallint,
                    _satellit_id integer[] DEFAULT NULL::integer[],
                    _since_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _until_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _tiles character varying[] DEFAULT NULL::character varying[],
                    _get_nodes boolean DEFAULT false)
                  RETURNS SETOF json AS
                $BODY$
                    DECLARE q text;
                    BEGIN
                        q := $sql$
                        WITH
                        product_type_names(id, name, description, row, is_raster) AS (
                        select id, name, description, row_number() over (order by description), is_raster
                        from product_type
                        -- LPIS products should be excluded
                        where name != 'lpis'
                        ),
                    $sql$;

                    IF $9 IS TRUE THEN
                        q := q || $sql$
                        site_names(id, name, geog, row) AS (
                        select s.id, s.name, st_astext(s.geog), row_number() over (order by s.name)
                        from site s
                        join public.user u on u.login = $1 and (u.role_id = 1 or s.id in (select * from unnest(u.site_id)))
                        ),
                        data(id, product, footprint, site_coord, product_type_id, satellite_id, is_raster) AS (

                        SELECT
                        P.id,
                        P.name,
                        P.footprint,
                        S.geog,
                        PT.id,
                        P.satellite_id,
                        PT.is_raster
                        $sql$;
                        ELSE
                        q := q || $sql$
                        site_names(id, name,  row) AS (
                        select id, name, row_number() over (order by name)
                        from site
                        ),
                          data(id, satellite_id, product_type_id, product_type_description,site, site_id) AS (
                        SELECT
                        P.id,
                        P.satellite_id,
                        PT.id,
                        PT.description,
                        S.name,
                        S.id
                         $sql$;
                    END IF;

                     q := q || $sql$
                        FROM product P
                        JOIN product_type_names PT ON P.product_type_id = PT.id
                        JOIN processor PR ON P.processor_id = PR.id
                        JOIN site_names S ON P.site_id = S.id
                        WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
                        AND EXISTS (
                            SELECT * FROM season WHERE season.site_id = P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date + interval '1 day'
                        $sql$;
                        IF $4 IS NOT NULL THEN
                        q := q || $sql$
                        AND season.id=$4
                        $sql$;
                        END IF;

                        q := q || $sql$
                        )
                        $sql$;
                         raise notice '%', _site_id;raise notice '%', _product_type_id;raise notice '%', _satellit_id;
                        IF $2 IS NOT NULL THEN
                        q := q || $sql$
                        AND P.site_id = ANY($2)

                        $sql$;
                        END IF;

                        IF $3 IS NOT NULL THEN
                        q := q || $sql$
                        AND P.product_type_id= ANY($3)

                        $sql$;
                        END IF;

                    IF $6 IS NOT NULL THEN
                    q := q || $sql$
                        AND P.created_timestamp >= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS')
                        $sql$;
                    END IF;

                    IF $7 IS NOT NULL THEN
                    q := q || $sql$
                        AND P.created_timestamp <= to_timestamp(cast($7 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day'
                        $sql$;
                    END IF;

                    IF $8 IS NOT NULL THEN
                    q := q || $sql$
                        AND P.tiles <@$8 AND P.tiles!='{}'
                        $sql$;
                    END IF;

                    q := q || $sql$
                        ORDER BY S.row, PT.row, P.name
                        )
                    --         select * from data;
                        SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
                        $sql$;

                        raise notice '%', q;

                        RETURN QUERY
                        EXECUTE q
                        USING _user_name, _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles, _get_nodes;
                    END
                    $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION sp_get_dashboard_products_nodes(character varying, integer[], integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[], boolean)
                  OWNER TO admin;
            $str$;
            raise notice '%', _statement;
            execute _statement;  

            IF to_regclass('public.ix_downloader_history_product_name') IS NULL THEN
                 _statement := $str$   
                    create index ix_downloader_history_product_name on downloader_history(product_name);  
                $str$;
                raise notice '%', _statement;
                execute _statement; 
            END IF;
            IF to_regclass('public.ix_product_name') IS NULL THEN
                 _statement := $str$   
                    create index ix_product_name on product(name);                
                $str$;
                raise notice '%', _statement;
                execute _statement; 
            END IF;
            
            ALTER TABLE product_provenance DROP CONSTRAINT IF EXISTS fk_product_provenance_product_id;
            alter table product_provenance add constraint fk_product_provenance_product_id foreign key(product_id) references product(id) on delete cascade;
            ALTER TABLE product_provenance DROP CONSTRAINT IF EXISTS fk_product_provenance_parent_product_id;
            alter table product_provenance add constraint fk_product_provenance_parent_product_id foreign key(parent_product_id) references product(id) on delete cascade;

-- Update functions for updating the end timestamp of the job (also to correctly display the end date of the job in the monitoring tab)
            _statement := $str$                        
                CREATE OR REPLACE FUNCTION sp_mark_job_finished(
                IN _job_id int
                ) RETURNS void AS $$
                BEGIN

                    UPDATE job
                    SET status_id = 6, --Finished
                    status_timestamp = now(),
                    end_timestamp = now()
                    WHERE id = _job_id; 

                END;
                $$ LANGUAGE plpgsql;
            $str$;
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
                    status_timestamp = now(),
                    end_timestamp = now()
                    WHERE id = _job_id
                    AND status_id NOT IN (6, 7, 8); -- Finished or failed jobs can't be cancelled
                END;
                $$ LANGUAGE plpgsql;
            $str$;
            raise notice '%', _statement;
            execute _statement;                

            _statement := $str$                        
                CREATE OR REPLACE FUNCTION sp_mark_job_failed(
                IN _job_id int
                ) RETURNS void AS $$
                BEGIN
                    -- Remaining tasks should be cancelled; the task that has failed has already been marked as failed.
                    UPDATE task
                    SET status_id = 7, -- Cancelled
                    status_timestamp = now()
                    WHERE job_id = _job_id
                    AND status_id NOT IN (6, 7, 8); -- Finished, cancelled or failed tasks can't be cancelled

                    UPDATE job
                    SET status_id = 8, -- Error
                    status_timestamp = now(),
                    end_timestamp = now()
                    WHERE id = _job_id;

                END;
                $$ LANGUAGE plpgsql;
            $str$;
            raise notice '%', _statement;
            execute _statement; 

            if exists (select * from config where key = 'executor.module.path.lai-end-of-job') then
                _statement := $str$
                    UPDATE config SET value = '/usr/bin/true' WHERE key = 'executor.module.path.lai-end-of-job';
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            if not exists (select * from config where key = 'executor.module.path.lai-processor-end-of-job') then
                _statement := $str$
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-processor-end-of-job', NULL, '/usr/bin/true', '2020-04-24 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

        if exists (select * from meta where version in ('2.0.0-RC1')) then
            raise notice 'upgrading from beta_version to 1.0.1';
            
            _statement := $str$
                DELETE FROM config WHERE key = 'processor.l3b.l1c_availability_days';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.l1c_availability_days';
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.l1c_availability_days', NULL, '20', '2017-10-24 14:56:57.501918+02');
                INSERT INTO config_metadata VALUES ('processor.l3b.l1c_availability_days', 'Number of days before current scheduled date within we must have L1C processed (default 20)', 'int', false, 4);
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                UPDATE datasource SET specific_params = json_build_object('parameters', specific_params->'parameters'->'dsParameter');
            $str$;
            raise notice '%', _statement;
            execute _statement;

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
                if exists (select * from processor where id = 9 and short_name = 'lpis') then
                    _statement := $str$
                        update processor set id = 8 where id = 9 and short_name = 'lpis';
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

    -- L3B updates
                _statement := $str$                
                    UPDATE config SET VALUE = 'L3B' WHERE key = 'processor.l3b_lai.sub_products';
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
                    DELETE FROM config WHERE key = 'processor.s4c_l4c.filter_ids_path';
                    
                    DELETE FROM config WHERE key = 'processor.s4c_l4c.practices' and site_id is null;
                    DELETE FROM config WHERE key = 'processor.s4c_l4c.country' and site_id is null;
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
                    INSERT INTO config(key, value) VALUES ('processor.s4c_l4c.filter_ids_path', '/mnt/archive/agric_practices_files/{site}/{year}/ts_input_tables/FilterIds/Sen4CAP_L4C_FilterIds.csv');
                    
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
                    DELETE FROM config WHERE key = 'processor.s4c_l4b.working_dir';
                    DELETE FROM config WHERE key = 'processor.s4c_l4b.gen_shp_py_script';
                    
                    DELETE FROM config WHERE key = 'processor.s4c_l4b.start_date' and site_id is null;
                    DELETE FROM config WHERE key = 'processor.s4c_l4b.end_date'  and site_id is null;
                    
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
                    INSERT INTO config(key, value) VALUES ('processor.s4c_l4b.working_dir', '/mnt/archive/grassland_mowing_files/{site}/{year}/working_dir/');
                    
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
            end if;

            if exists (select * from meta where version in ('1.0.0')) then
                raise notice 'upgrading from 1.0.0 to 1.0.1';
                
                _statement := $str$            
                    CREATE OR REPLACE FUNCTION sp_get_l1c_products(
                        IN site_id smallint DEFAULT NULL::smallint,
                        IN sat_ids json DEFAULT NULL::json,
                        IN status_ids json DEFAULT NULL::json,
                        IN start_time timestamp with time zone DEFAULT NULL::timestamp with time zone,
                        IN end_time timestamp with time zone DEFAULT NULL::timestamp with time zone)
                      RETURNS TABLE("ProductId" integer, "Name" character varying, "SiteId" smallint, full_path character varying, "SatelliteId" smallint, "StatusId" smallint, product_date timestamp with time zone, created_timestamp timestamp with time zone) AS
                    $BODY$
                    DECLARE q text;
                    BEGIN
                        q := $sql$
                        WITH site_names(id, name, row) AS (
                                select id, name, row_number() over (order by name)
                                from site
                            ),
                            satellite_ids(id, name, row) AS (
                                select id, satellite_name, row_number() over (order by satellite_name)
                                from satellite
                            )
                            SELECT P.id AS ProductId,
                                P.product_name AS Name,
                                P.site_id as SiteId,
                                P.full_path,
                                P.satellite_id as SatelliteId,
                                P.status_id as StatusId,
                                P.product_date,
                                P.created_timestamp
                            FROM downloader_history P
                                JOIN satellite_ids SAT ON P.satellite_id = SAT.id
                                JOIN site_names S ON P.site_id = S.id
                            WHERE TRUE$sql$;

                        IF NULLIF($1, -1) IS NOT NULL THEN
                            q := q || $sql$
                                AND P.site_id = $1$sql$;
                        END IF;
                        IF $2 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.satellite_id IN (SELECT value::smallint FROM json_array_elements_text($2))
                            $sql$;
                        END IF;
                        IF $3 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.status_id IN (SELECT value::smallint FROM json_array_elements_text($3))
                            $sql$;
                        END IF;
                        IF $4 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.product_date >= $4$sql$;
                        END IF;
                        IF $5 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.product_date <= $5$sql$;
                        END IF;
                        q := q || $SQL$
                            ORDER BY S.row, SAT.row, P.product_name;$SQL$;

                        -- raise notice '%', q;
                        
                        RETURN QUERY
                            EXECUTE q
                            USING $1, $2, $3, $4, $5;
                    END
                    $BODY$
                      LANGUAGE plpgsql STABLE;
                $str$;
                raise notice '%', _statement;
                execute _statement;
                
                _statement := $str$            
                    drop function sp_get_job_output(_job_id job.id%type)
                $str$;
                raise notice '%', _statement;
                execute _statement;                

                _statement := $str$            
                    create or replace function sp_get_job_output(
                        _job_id job.id%type
                    ) returns table (
                        step_name step.name%type,
                        command text,
                        stdout_text step_resource_log.stdout_text%type,
                        stderr_text step_resource_log.stderr_text%type,
                        exit_code step.exit_code%type,
                        execution_status step.status_id%type
                    ) as
                    $$
                    begin
                        return query
                            select step.name,
                                   array_to_string(array_prepend(config.value :: text, array(select json_array_elements_text(json_extract_path(step.parameters, 'arguments')))), ' ') as command,
                                   step_resource_log.stdout_text,
                                   step_resource_log.stderr_text,
                                   step.exit_code,
                                   step.status_id
                            from task
                            inner join step on step.task_id = task.id
                            left outer join step_resource_log on step_resource_log.step_name = step.name and step_resource_log.task_id = task.id
                            left outer join config on config.site_id is null and config.key = 'executor.module.path.' || task.module_short_name
                            where task.job_id = _job_id
                            order by step.submit_timestamp;
                    end;
                    $$
                        language plpgsql stable;
                $str$;
                raise notice '%', _statement;
                execute _statement;                
            end if;
            
            _statement := 'update meta set version = ''1.1'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
