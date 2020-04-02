begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('2.0.1', '2.0.2')) then
            if exists (select * from meta where version = '2.0.1') then
                raise notice 'upgrading from 2.0.1 to 2.0.2';

                raise notice 'patching 2.0.1';
                
                if not exists (select * from config where key = 'downloader.use.esa.l2a') then
                    _statement := $str$
                        INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.use.esa.l2a', NULL, 'false', '2019-12-16 14:56:57.501918+02');
                        INSERT INTO config_metadata VALUES ('downloader.use.esa.l2a', 'Enable S2 L2A ESA products download', 'bool', false, 15);
                    $str$;
                    raise notice '%', _statement;
                    execute _statement;
                end if;
                
                _statement := $str$
                    UPDATE datasource SET specific_params = json_build_object('parameters', specific_params->'parameters'->'dsParameter');
                $str$;
                raise notice '%', _statement;
                execute _statement;

    -- Switch LAI production to INRA version
                _statement := $str$
                    UPDATE config SET VALUE = '/usr/share/sen2agri/Lai_Bands_Cfgs_Belcam.cfg' WHERE KEY = 'processor.l3b.lai.laibandscfgfile';
                    UPDATE config SET VALUE = '1' WHERE KEY = 'processor.l3b.lai.use_inra_version';
                    
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.l1c_availability_days', NULL, '20', '2017-10-24 14:56:57.501918+02');
                    INSERT INTO config_metadata VALUES ('processor.l3b.l1c_availability_days', 'Number of days before current scheduled date within we must have L1C processed (default 20)', 'int', false, 4);
                    
                $str$;
                raise notice '%', _statement;
                execute _statement;

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
            
            end if;
            
            if exists (select * from meta where version = '2.0.1') then
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
            
            _statement := 'update meta set version = ''2.0.2'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;
    raise notice 'complete';
end;
$migration$;

commit;
