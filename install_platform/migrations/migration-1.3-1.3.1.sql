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

        -- 04b66cb98a160882120e579fd38ea685191e916d
        raise notice 'applying 04b66cb98a160882120e579fd38ea685191e916d';
        raise notice 'update config set key = ''executor.module.path.dimensionality-reduction'', value=''otbcli_DimensionalityReduction'' where key = ''executor.module.path.principal-component-analysis'';';
        update config set key = 'executor.module.path.dimensionality-reduction', value='otbcli_DimensionalityReduction' where key = 'executor.module.path.principal-component-analysis';
        raise notice 'update config_metadata set key = ''executor.module.path.dimensionality-reduction'', friendly_name=''Dimensionality reduction'' where key = ''executor.module.path.principal-component-analysis'';';
        update config_metadata set key = 'executor.module.path.dimensionality-reduction', friendly_name='Dimensionality reduction' where key = 'executor.module.path.principal-component-analysis';

        -- b57b113cc28e06ae9c72aecd5be0cfca5dcfa833
        raise notice 'applying b57b113cc28e06ae9c72aecd5be0cfca5dcfa833';
        raise notice 'update config set value=''40000'' where key = ''processor.l4a.training-samples-number'';';
        update config set value='40000' where key = 'processor.l4a.training-samples-number';
        raise notice 'update config set value=''resample'' where key in (''processor.l4a.temporal_resampling_mode'', ''processor.l4b.temporal_resampling_mode'');';
        update config set value='resample' where key in ('processor.l4a.temporal_resampling_mode', 'processor.l4b.temporal_resampling_mode');

        -- bc8471f8275052acf1a0812861b967a4c0a60156
        raise notice 'applying bc8471f8275052acf1a0812861b967a4c0a60156';
        raise notice 'create or replace function sp_get_products( [...] );';
        CREATE OR REPLACE FUNCTION sp_get_products(
            IN site_id smallint DEFAULT NULL::smallint,
            IN product_type_id smallint DEFAULT NULL::smallint,
            IN start_time timestamp with time zone DEFAULT NULL::timestamp with time zone,
            IN end_time timestamp with time zone DEFAULT NULL::timestamp with time zone)
        RETURNS TABLE("ProductId" integer, "Product" character varying, "ProductType" character varying, "ProductTypeId" smallint, "Processor" character varying,
                        "ProcessorId" smallint, "Site" character varying, "SiteId" smallint, full_path character varying,
                        quicklook_image character varying, footprint polygon, created_timestamp timestamp with time zone) AS
        $BODY$
        DECLARE q text;
        BEGIN
            q := $sql$
            WITH site_names(id, name, row) AS (
                    select id, name, row_number() over (order by name)
                    from site
                ),
                product_type_names(id, name, row) AS (
                    select id, name, row_number() over (order by name)
                    from product_type
                )
                SELECT P.id AS ProductId,
                    P.name AS Product,
                    PT.name AS ProductType,
                    P.product_type_id AS ProductTypeId,
                    PR.name AS Processor,
                    P.processor_id AS ProcessorId,
                    S.name AS Site,
                    P.site_id AS SiteId,
                    P.full_path,
                    P.quicklook_image,
                    P.footprint,
                    P.created_timestamp
                FROM product P
                    JOIN product_type_names PT ON P.product_type_id = PT.id
                    JOIN processor PR ON P.processor_id = PR.id
                    JOIN site_names S ON P.site_id = S.id
                WHERE TRUE$sql$;

            IF NULLIF($1, -1) IS NOT NULL THEN
                q := q || $sql$
                    AND P.site_id = $1$sql$;
            END IF;
            IF NULLIF($2, -1) IS NOT NULL THEN
                q := q || $sql$
                    AND P.product_type_id = $2$sql$;
            END IF;
            IF $3 IS NOT NULL THEN
                q := q || $sql$
                    AND P.created_timestamp >= $3$sql$;
            END IF;
            IF $4 IS NOT NULL THEN
                q := q || $sql$
                    AND P.created_timestamp <= $4$sql$;
            END IF;
            q := q || $SQL$
                ORDER BY S.row, PT.row, P.name;$SQL$;

            -- raise notice '%', q;

            RETURN QUERY
                EXECUTE q
                USING $1, $2, $3, $4;
        END
        $BODY$
        LANGUAGE plpgsql STABLE;

        raise notice 'DROP FUNCTION sp_get_dashboard_products(smallint, smallint);';
        DROP FUNCTION sp_get_dashboard_products(smallint, smallint);

        raise notice 'create or replace function sp_get_dashboard_products( [...] );';
        -- Function: sp_get_dashboard_products(smallint, smallint)

        -- DROP FUNCTION sp_get_dashboard_products(smallint, smallint);

        CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
            site_id smallint DEFAULT NULL::smallint,
            processor_id smallint DEFAULT NULL::smallint)
        RETURNS TABLE (
        --     id product.id%type,
        --     satellite_id product.satellite_id%type,
        --     product product.name%type,
        --     product_type product_type.name%type,
        --     product_type_description product_type.description%type,
        --     processor processor.name%type,
        --     site site.name%type,
        --     full_path product.full_path%type,
        --     quicklook_image product.quicklook_image%type,
        --     footprint product.footprint%type,
        --     created_timestamp product.created_timestamp%type,
        --     site_coord text
            json json
        ) AS
        $BODY$
        DECLARE q text;
        BEGIN
            q := $sql$
                WITH site_names(id, name, geog, row) AS (
                        select id, name, st_astext(geog), row_number() over (order by name)
                        from site
                    ),
                    product_type_names(id, name, description, row) AS (
                        select id, name, description, row_number() over (order by description)
                        from product_type
                    ),
                    data(id, satellite_id, product,product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
                    SELECT
                        P.id,
                        P.satellite_id,
                        P.name,
                        PT.name,
                        PT.description,
                        PR.name,
                        S.name,
                        P.full_path,
                        P.quicklook_image,
                        P.footprint,
                        P.created_timestamp,
                        S.geog
                    FROM product P
                        JOIN product_type_names PT ON P.product_type_id = PT.id
                        JOIN processor PR ON P.processor_id = PR.id
                        JOIN site_names S ON P.site_id = S.id
                    WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
            $sql$;
            IF $1 IS NOT NULL THEN
                q := q || 'AND P.site_id = $1';
            END IF;
            IF $2 IS NOT NULL THEN
                q := q || 'AND P.product_type_id = $2';
            END IF;

            q := q || $sql$
                    ORDER BY S.row, PT.row, P.name
                )
        --         select * from data;
                SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
            $sql$;

        --     raise notice '%', q;

            RETURN QUERY
            EXECUTE q
            USING $1, $2;
        END
        $BODY$
        LANGUAGE plpgsql STABLE
        COST 100;
        ALTER FUNCTION sp_get_dashboard_products(smallint, smallint)
        OWNER TO admin;

        raise notice 'insert into meta(version) values(''1.3.1'');';
        insert into meta(version) values('1.3.1');

    end if;

    raise notice 'complete';
end;
$migration$;

commit;
