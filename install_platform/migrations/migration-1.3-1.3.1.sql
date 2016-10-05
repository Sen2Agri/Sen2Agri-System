begin transaction;

do $migration$
begin
    raise notice 'running migrations';

    if not exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        raise notice 'upgrading from 1.3 to 1.3.1';

        -- fe830351038209badcd9bd2daee36501a2cb1ac8
        raise notice 'applying fe830351038209badcd9bd2daee36501a2cb1ac8';
        if not exists (select * from config where key = 'executor.module.path.files-remover' and site_id is null) then
            raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''executor.module.path.files-remover'', NULL, ''/usr/bin/rm'', ''2015-08-24 17:44:38.29255+03'');';
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.files-remover', NULL, '/usr/bin/rm', '2015-08-24 17:44:38.29255+03');
        end if;

        if not exists (select * from config_metadata where key = 'executor.module.path.files-remover') then
            raise notice 'INSERT INTO config_metadata VALUES (''executor.module.path.files-remover'', ''Removes the given files (ex. cleanup of intermediate files)'', ''file'', true, 8);';
            INSERT INTO config_metadata VALUES ('executor.module.path.files-remover', 'Removes the given files (ex. cleanup of intermediate files)', 'file', true, 8);
        end if;

        -- 9e5a4037d89de6bea3dc8e8c47553b1ec978cd5e
        raise notice 'applying 9e5a4037d89de6bea3dc8e8c47553b1ec978cd5e';
        if exists (select * from config_metadata where key = 'processor.l3a.weight.total.bandsmapping') then
            raise notice 'delete from config_metadata where key = ''processor.l3a.weight.total.bandsmapping'';';
            delete from config_metadata where key = 'processor.l3a.weight.total.bandsmapping';
        end if;

        -- 4b3ad4050e4812ac007dfa59d4e334c49e370a51
        raise notice 'applying 4b3ad4050e4812ac007dfa59d4e334c49e370a51';
        raise notice 'drop function if exists sp_get_job_history(smallint, integer);';
        drop function if exists sp_get_job_history(smallint, integer);

        raise notice 'CREATE OR REPLACE FUNCTION sp_get_job_history_custom_page(smallint, integer, integer) [...];';
        CREATE OR REPLACE FUNCTION sp_get_job_history_custom_page(
            IN _siteid smallint DEFAULT NULL::smallint,
            IN _page integer DEFAULT 1,
            IN _rows_per_page integer DEFAULT 20)
          RETURNS TABLE(id integer, end_timestamp timestamp with time zone, processor character varying, site character varying, status character varying, start_type character varying) AS
        $BODY$
        BEGIN
            RETURN QUERY
                SELECT J.id, J.end_timestamp, P.name, S.name, AST.name, ST.name
                       FROM job J
                               JOIN processor P ON J.processor_id = P.id
                               JOIN site S ON J.site_id = S.id
                               JOIN job_start_type ST ON J.start_type_id = ST.id
                               JOIN activity_status AST ON J.status_id = AST.id
                       WHERE   $1 IS NULL OR site_id = _siteid
               ORDER BY J.end_timestamp DESC
               OFFSET ($2 - 1) * _rows_per_page LIMIT _rows_per_page;
        END
        $BODY$
          LANGUAGE plpgsql STABLE
          COST 100
          ROWS 1000;
        ALTER FUNCTION sp_get_job_history_custom_page(smallint, integer, integer)
          OWNER TO admin;

        -- 9c2ded798784fd992dc751d99358894c48126341
        raise notice 'applying 9c2ded798784fd992dc751d99358894c48126341';
        raise notice 'update config set value = ''0'' where key like ''executor.processor.%.keep_job_folders'' and site_id is null;', '%';
        update config set value = '0' where key like 'executor.processor.%.keep_job_folders' and site_id is null;

        -- ae11bd7027053395846163e7033788cc1d8c551e
        -- skipped, the template cannot be changed after the database was created

        -- 7cf9e2e94fa2e55161ebeb651b69a2f862eb3402
        raise notice 'applying 7cf9e2e94fa2e55161ebeb651b69a2f862eb3402';
        IF NOT EXISTS (select * from pg_indexes where schemaname = 'public' and tablename = 'config' and indexname = 'ix_config_key_site_id') THEN
            raise notice 'CREATE UNIQUE INDEX ix_config_key_site_id ON config("key", COALESCE(site_id, -1));';
            CREATE UNIQUE INDEX ix_config_key_site_id ON config("key", COALESCE(site_id, -1));
        END IF;

        -- 2fd3c73a2cad8919ad9e1e22ddc11788e0420c63
        raise notice 'applying 2fd3c73a2cad8919ad9e1e22ddc11788e0420c63';
        if not exists (select * from config where key = 'executor.processor.l3a.slurm_qos' and site_id is null) then
            raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''executor.processor.l3a.slurm_qos'', NULL, ''qoscomposite'', ''2015-08-24 17:44:38.29255+03'');';
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l3a.slurm_qos', NULL, 'qoscomposite', '2015-08-24 17:44:38.29255+03');
        end if;
        if not exists (select * from config where key = 'executor.processor.l3b_lai.slurm_qos' and site_id is null) then
            raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''executor.processor.l3b_lai.slurm_qos'', NULL, ''qoslai'', ''2015-08-24 17:44:38.29255+03'');';
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l3b_lai.slurm_qos', NULL, 'qoslai', '2015-08-24 17:44:38.29255+03');
        end if;
        if not exists (select * from config where key = 'executor.processor.l3e_pheno.slurm_qos' and site_id is null) then
            raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''executor.processor.l3e_pheno.slurm_qos'', NULL, ''qospheno'', ''2015-08-24 17:44:38.29255+03'');';
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l3e_pheno.slurm_qos', NULL, 'qospheno', '2015-08-24 17:44:38.29255+03');
        end if;
        if not exists (select * from config where key = 'executor.processor.l4a.slurm_qos' and site_id is null) then
            raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''executor.processor.l4a.slurm_qos'', NULL, ''qoscropmask'', ''2015-08-24 17:44:38.29255+03'');';
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l4a.slurm_qos', NULL, 'qoscropmask', '2015-08-24 17:44:38.29255+03');
        end if;
        if not exists (select * from config where key = 'executor.processor.l4b.slurm_qos' and site_id is null) then
            raise notice 'INSERT INTO config(key, site_id, value, last_updated) VALUES (''executor.processor.l4b.slurm_qos'', NULL, ''qoscroptype'', ''2015-08-24 17:44:38.29255+03'');';
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l4b.slurm_qos', NULL, 'qoscroptype', '2015-08-24 17:44:38.29255+03');
        end if;

        if not exists (select * from config_metadata where key = 'executor.processor.l3a.slurm_qos') then
            raise notice 'INSERT INTO config_metadata VALUES (''executor.processor.l3a.slurm_qos'', ''Slurm QOS for composite processor'', ''string'', true, 8);';
            INSERT INTO config_metadata VALUES ('executor.processor.l3a.slurm_qos', 'Slurm QOS for composite processor', 'string', true, 8);
        end if;
        if not exists (select * from config_metadata where key = 'executor.processor.l3b_lai.slurm_qos') then
            raise notice 'INSERT INTO config_metadata VALUES (''executor.processor.l3b_lai.slurm_qos'', ''Slurm QOS for LAI processor'', ''string'', true, 8);';
            INSERT INTO config_metadata VALUES ('executor.processor.l3b_lai.slurm_qos', 'Slurm QOS for LAI processor', 'string', true, 8);
        end if;
        if not exists (select * from config_metadata where key = 'executor.processor.l3e_pheno.slurm_qos') then
            raise notice 'INSERT INTO config_metadata VALUES (''executor.processor.l3e_pheno.slurm_qos'', ''Slurm QOS for Pheno NDVI processor'', ''string'', true, 8);';
            INSERT INTO config_metadata VALUES ('executor.processor.l3e_pheno.slurm_qos', 'Slurm QOS for Pheno NDVI processor', 'string', true, 8);
        end if;
        if not exists (select * from config_metadata where key = 'executor.processor.l4a.slurm_qos') then
            raise notice 'INSERT INTO config_metadata VALUES (''executor.processor.l4a.slurm_qos'', ''Slurm QOS for CropMask processor'', ''string'', true, 8);';
            INSERT INTO config_metadata VALUES ('executor.processor.l4a.slurm_qos', 'Slurm QOS for CropMask processor', 'string', true, 8);
        end if;
        if not exists (select * from config_metadata where key = 'executor.processor.l4b.slurm_qos') then
            raise notice 'INSERT INTO config_metadata VALUES (''executor.processor.l4b.slurm_qos'', ''Slurm QOS for CropType processor'', ''string'', true, 8);';
            INSERT INTO config_metadata VALUES ('executor.processor.l4b.slurm_qos', 'Slurm QOS for CropType processor', 'string', true, 8);
        end if;

        -- d549862f25a42ac10178374872628bfefe83cb50
        raise notice 'applying d549862f25a42ac10178374872628bfefe83cb50';
        raise notice 'create table if not exists meta([...]);';
        CREATE TABLE IF NOT EXISTS meta (
            version text NOT NULL
        );

        raise notice 'insert into meta(version) values(''1.3.1'');';
        insert into meta(version) values('1.3.1');

    end if;

    raise notice 'complete';
end;
$migration$;

commit;
