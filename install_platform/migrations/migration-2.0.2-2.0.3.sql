begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('2.0.2', '2.0.3')) then
            if exists (select * from meta where version = '2.0.2') then
                raise notice 'upgrading from 2.0.2 to 2.0.3';

                raise notice 'patching 2.0.2';
                
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

                $str$;
                raise notice '%', _statement;
                execute _statement;
            
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
            
            _statement := 'update meta set version = ''2.0.3'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;
    raise notice 'complete';
end;
$migration$;

commit;
