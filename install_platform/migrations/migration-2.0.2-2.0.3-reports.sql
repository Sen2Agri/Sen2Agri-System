begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('2.0.2')) then
            -- new tables creation, if not exist 
            _statement := $str$
                CREATE SCHEMA IF NOT EXISTS reports;

                CREATE TABLE IF NOT EXISTS reports.s1_report
                (
                  downloader_history_id integer,
                  orbit_id smallint,
                  acquisition_date date,
                  product_name character varying,
                  status_description character varying,
                  intersection_date date,
                  intersected_status_id smallint,
                  intersection numeric(5,2),
                  polarisation character varying,
                  l2_product character varying,
                  l2_coverage numeric(5,2),
                  status_reason character varying,
                  intersected_product character varying,
                  site_id smallint
                )
                WITH (
                  OIDS=FALSE
                );
                ALTER TABLE reports.s1_report
                  OWNER TO postgres;

                CREATE TABLE IF NOT EXISTS reports.s2_report
                (
                  site_id smallint,
                  downloader_history_id integer,
                  orbit_id smallint,
                  acquisition_date date,
                  product_name character varying,
                  status_description character varying,
                  status_reason character varying,
                  l2_product character varying,
                  clouds integer
                )
                WITH (
                  OIDS=FALSE
                );
                ALTER TABLE reports.s2_report
                  OWNER TO postgres;

                CREATE TABLE IF NOT EXISTS reports.l8_report
                (
                  site_id smallint,
                  downloader_history_id integer,
                  orbit_id character varying,
                  acquisition_date date,
                  product_name character varying,
                  status_description character varying,
                  status_reason character varying,
                  l2_product character varying,
                  clouds integer
                )
                WITH (
                  OIDS=FALSE
                );
                ALTER TABLE reports.l8_report
                  OWNER TO postgres;
                  
                CREATE OR REPLACE FUNCTION reports.sp_get_l8_statistics(IN site_id smallint)
                  RETURNS TABLE("site" smallint, "downloader_history_id" integer, "orbit_id" integer, "acquisition_date" date, "acquisition" character varying, "acquisition_status" character varying, 
                        status_reason character varying, l2_product character varying, clouds integer) AS
                $BODY$

                BEGIN
                    RETURN QUERY
                    SELECT 	$1 as site_id,
                        d.id,
                        split_part(d.product_name, '_', 3)::integer as orbit, 
                        to_date(split_part(d.product_name, '_', 4), 'YYYYMMDD') as acquisition_date, 
                        d.product_name as acquisition, 
                        ds.status_description as status, 
                        case 	when d.status_id in (1,2,3,4,41,5) then d.status_reason
                            when d.status_id in (6,7,8) then concat('clouds:',coalesce(th.cloud_coverage::varchar,'n/a'),'; snow:',coalesce(th.snow_coverage::varchar,'n/a'),'; failure:',coalesce(regexp_replace(th.failed_reason,E'[\\n]+',' ','g'),'n/a'))
                            else null end as status_reason, 
                        p.name as l2_product, 
                        coalesce(th.cloud_coverage, -1) as clouds 
                    FROM public.downloader_history d
                        JOIN public.downloader_status ds ON ds.id = d.status_id 
                        LEFT JOIN public.l1_tile_history th ON th.downloader_history_id = d.id 
                        LEFT JOIN public.product p ON REPLACE(p.name, '_L2A_', '_L1TP_') = d.product_name 
                    WHERE NOT EXISTS(SELECT sr.* FROM reports.l8_report sr WHERE sr.downloader_history_id = d.id) AND d.satellite_id = 2 and d.site_id = $1
                    ORDER BY d.orbit_id, acquisition_date, d.product_name, l2_product;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_get_l8_statistics(smallint)
                  OWNER TO postgres;
                  
                  
                CREATE OR REPLACE FUNCTION reports.sp_get_s1_statistics(IN site_id smallint)
                  RETURNS TABLE("site" smallint, "downloader_history_id" integer, "orbit_id" integer, "acquisition_date" date, "acquisition" character varying, "acquisition_status" character varying, 
                        "intersection_date" date, "intersected_product" character varying, "intersected_status" smallint, "intersection" double precision,
                        "polarisation" character varying, l2_product character varying, l2_coverage double precision, status_reason character varying) AS
                $BODY$

                BEGIN
                    RETURN QUERY
                    WITH d AS (select dh.*,ds.status_description from public.downloader_history dh join public.downloader_status ds on ds.id = dh.status_id)
                select 	$1 as site,
                    d.id,
                    d.orbit_id as orbit, 
                    to_date(substr(split_part(d.product_name, '_', 6), 1, 8),'YYYYMMDD') as acquisition_date, 
                    d.product_name as acquisition,
                    d.status_description as acquisition_status,
                    to_date(substr(split_part(i.product_name, '_', 6), 1, 8),'YYYYMMDD') as intersection_date,
                    i.product_name as intersected_product,
                    i.status_id as intersected_status,
                    st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) * 100 as intersection,
                    split_part(p.name, '_', 6)::character varying as polarisation,
                    p.name as l2_product,
                    st_area(st_intersection(d.footprint, p.geog))/st_area(d.footprint) * 100 as l2_coverage,
                    d.status_reason
                    from d
                    join public.downloader_history i on i.site_id = d.site_id AND i.orbit_id = d.orbit_id AND i.satellite_id = d.satellite_id and st_intersects(d.footprint, i.footprint) AND DATE_PART('day', d.product_date - i.product_date) BETWEEN 5 AND 7 AND st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) > 0.05
                    join public.product p on p.downloader_history_id = d.id
                    WHERE NOT EXISTS(SELECT sr.* FROM reports.s1_report sr WHERE sr.downloader_history_id = d.id AND sr.intersected_product = i.product_name AND sr.site_id = i.site_id AND sr.l2_product = p.name)
                        and d.site_id = $1 AND d.satellite_id = 3 and i.id is not null
                        and p.name like concat('%', substr(split_part(i.product_name, '_', 6), 1, 15),'%')
                union
                select 	$1 as site,
                    d.id,
                    d.orbit_id as orbit, 
                    to_date(substr(split_part(d.product_name, '_', 6), 1, 8),'YYYYMMDD') as acquisition_date, 
                    d.product_name as acquisition,
                    d.status_description as acquisition_status,
                    to_date(substr(split_part(i.product_name, '_', 6), 1, 8),'YYYYMMDD') as intersection_date,
                    i.product_name as intersected_product,
                    i.status_id as intersected_status,
                    st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) * 100 as intersection,
                    split_part(p.name, '_', 6)::character varying as polarisation,
                    p.name as l2_product,
                    st_area(st_intersection(d.footprint, p.geog))/st_area(d.footprint) * 100 as l2_coverage,
                    d.status_reason
                    from d
                    join public.downloader_history i on i.site_id = d.site_id AND i.orbit_id = d.orbit_id AND i.satellite_id = d.satellite_id and st_intersects(d.footprint, i.footprint) AND DATE_PART('day', d.product_date - i.product_date) BETWEEN 11 AND 13 AND st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) > 0.05
                    join public.product p on p.downloader_history_id = d.id
                    left outer join public.product_stats ps on ps.product_id = p.id
                    WHERE NOT EXISTS(SELECT sr.* FROM reports.s1_report sr WHERE sr.downloader_history_id = d.id AND sr.intersected_product = i.product_name AND sr.site_id = i.site_id AND sr.l2_product = p.name)
                        and d.site_id = $1 AND d.satellite_id = 3 and i.id is not null and left(d.product_name, 3) = left(i.product_name, 3)
                        and p.name like concat('%', substr(split_part(i.product_name, '_', 6), 1, 15),'%')
                union
                select 	$1 as site,
                    d.id,
                    d.orbit_id as orbit,
                    to_date(substr(split_part(d.product_name, '_', 6), 1, 8),'YYYYMMDD') as acquisition_date, 
                    d.product_name as acquisition,
                    ds.status_description as acquisition_status,
                    to_date(substr(split_part(i.product_name, '_', 6), 1, 8),'YYYYMMDD') as intersection_date,
                    i.product_name as intersected_product,
                    i.status_id as intersected_status,
                    case when i.footprint is null then null else st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) * 100 end as intersection,
                    null as polarisation,
                    null as l2_product,
                    null as l2_coverage,
                    null as status_reason
                    from public.downloader_history d
                        join public.downloader_status ds on ds.id = d.status_id
                        left outer join public.downloader_history i on i.site_id = d.site_id AND i.orbit_id = d.orbit_id AND i.satellite_id = d.satellite_id and st_intersects(d.footprint, i.footprint) AND DATE_PART('day', d.product_date - i.product_date) BETWEEN 5 AND 7 AND st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) > 0.05
                    where NOT EXISTS(SELECT sr.* FROM reports.s1_report sr WHERE sr.downloader_history_id = d.id) and d.site_id = $1 AND d.satellite_id = 3 and d.status_id != 5;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_get_s1_statistics(smallint)
                  OWNER TO postgres;
                  
                  
                  
                CREATE OR REPLACE FUNCTION reports.sp_get_s2_statistics(IN site_id smallint)
                  RETURNS TABLE("site" smallint, "downloader_history_id" integer, "orbit_id" integer, "acquisition_date" date, "acquisition" character varying, "acquisition_status" character varying, 
                        status_reason character varying, l2_product character varying, clouds integer) AS
                $BODY$

                BEGIN
                    RETURN QUERY
                    SELECT $1 as site_id,
                    d.id,	
                    coalesce(d.orbit_id, (d.tiles::text)::integer) as orbit, 
                    to_date(substr(split_part(d.product_name, '_', 3), 1, 8), 'YYYYMMDD') as acquisition_date, 
                    d.product_name as acquisition, 
                    ds.status_description as status,
                    case 	when d.status_id = 5 and coalesce(th.status_id,0) = 2 then 'failed l1_tile_history'
                        when d.status_id in (2,7) and coalesce(th.status_id,0) = 3 then 'processed l1_tile_history'
                        when d.status_id = 1 and d.full_path like '%$value' then 'not found'
                        when d.status_id in (1,2,3,4,41,5) then d.status_reason
                        when d.status_id in (6,7,8) then concat('clouds:',coalesce(th.cloud_coverage::varchar,'n/a'),'; snow:',coalesce(th.snow_coverage::varchar,'n/a'),'; failure:',coalesce(regexp_replace(th.failed_reason,E'[\\n]+',' ','g'),'n/a'))
                        else null end as status_reason,
                    p.name as l2_product,
                    coalesce(th.cloud_coverage, -1) as clouds
                    FROM public.downloader_history d 
                        JOIN public.downloader_status ds ON ds.id = d.status_id
                        LEFT JOIN public.l1_tile_history th ON th.downloader_history_id = d.id 
                        LEFT JOIN public.product p ON p.name IN (d.product_name, REPLACE(d.product_name, 'MSIL1C', 'MSIL2A'))
                    WHERE NOT EXISTS(SELECT sr.* FROM reports.s2_report sr WHERE sr.downloader_history_id = d.id) AND d.satellite_id = 1 and d.site_id = $1
                ORDER BY d.orbit_id, acquisition_date, d.product_name, l2_product;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_get_s2_statistics(smallint)
                  OWNER TO postgres;



                -- Function: reports.sp_insert_l8_statistics()
                -- DROP FUNCTION reports.sp_insert_l8_statistics();
                CREATE OR REPLACE FUNCTION reports.sp_insert_l8_statistics()
                  RETURNS TABLE(rows integer) AS
                $BODY$

                BEGIN
                    RETURN QUERY
                    WITH rows AS (
                    INSERT INTO reports.l8_report(site_id, downloader_history_id, orbit_id, acquisition_date, product_name, status_description, status_reason, l2_product, clouds)
                    SELECT (f).* FROM (SELECT reports.sp_get_l8_statistics(id) as f FROM public.site where enabled = true) from_site
                    RETURNING 1)
                    SELECT COUNT(*)::integer FROM rows;
                END

                $BODY$
                  LANGUAGE plpgsql VOLATILE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_insert_l8_statistics()
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_insert_s1_statistics()
                  RETURNS TABLE("rows" integer) AS
                $BODY$

                BEGIN
                    RETURN QUERY
                    WITH rows AS (
                    INSERT INTO reports.s1_report(site_id,
                            downloader_history_id, orbit_id, acquisition_date, product_name, 
                            status_description, intersection_date, intersected_product, intersected_status_id, 
                            intersection, polarisation, l2_product, l2_coverage, status_reason)
                    SELECT (f).* FROM (SELECT reports.sp_get_s1_statistics(id) as f FROM public.site where enabled = true) from_site
                    RETURNING 1)
                    SELECT COUNT(*)::integer FROM rows;
                END

                $BODY$
                  LANGUAGE plpgsql VOLATILE
                  COST 100;
                ALTER FUNCTION reports.sp_insert_s1_statistics()
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_insert_s2_statistics()
                  RETURNS TABLE("rows" integer) AS
                $BODY$

                BEGIN
                    RETURN QUERY
                    WITH rows AS (
                    INSERT INTO reports.s2_report(site_id, downloader_history_id, orbit_id, acquisition_date, product_name, status_description, status_reason, l2_product, clouds)
                    SELECT (f).* FROM (SELECT reports.sp_get_s2_statistics(id) as f FROM public.site where enabled = true) from_site
                    RETURNING 1)
                    SELECT COUNT(*)::integer FROM rows;
                END

                $BODY$
                  LANGUAGE plpgsql VOLATILE
                  COST 100;
                ALTER FUNCTION reports.sp_insert_s2_statistics()
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_l8_detail(
                    siteid smallint DEFAULT NULL::smallint,
                    orbitid integer DEFAULT NULL::integer,
                    fromdate date DEFAULT NULL::date,
                    todate date DEFAULT NULL::date)
                RETURNS TABLE(site_id smallint, downloader_history_id integer, orbit_id integer, acquisition_date date, product_name character varying, status_description character varying, status_reason character varying, l2_product character varying, clouds integer) 
                    LANGUAGE 'plpgsql'
                    COST 100
                    STABLE 
                    ROWS 1000
                AS $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT MIN(r.acquisition_date) INTO startDate FROM reports.l8_report r;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT MAX(r.acquisition_date) INTO endDate FROM reports.l8_report r;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    SELECT r.site_id, r.downloader_history_id, r.orbit_id::integer, r.acquisition_date, r.product_name, r.status_description, r.status_reason, r.l2_product, r.clouds 
                        FROM reports.l8_report r
                        WHERE ($1 IS NULL OR r.site_id = $1) AND ($2 IS NULL OR r.orbit_id::integer = $2) AND r.acquisition_date BETWEEN startDate AND endDate
                    ORDER BY r.site_id, r.acquisition_date, r.product_name;
                END


                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_l8_detail(smallint, integer, date, date)
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_l8_statistics(
                    IN siteId smallint DEFAULT NULL::smallint,
                    IN orbitId integer DEFAULT NULL::integer,
                    IN fromDate date DEFAULT NULL::date,
                    IN toDate date DEFAULT NULL::date)
                  RETURNS TABLE(calendar_date date, acquisitions integer, failed_to_download integer, processed integer, not_yet_processed integer, falsely_processed integer, errors integer, clouds integer) AS
                $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT MIN(acquisition_date) INTO startDate FROM reports.l8_report;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT MAX(acquisition_date) INTO endDate FROM reports.l8_report;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    WITH 	calendar AS 
                            (SELECT date_trunc('day', dd)::date AS cdate 
                                FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
                        ac AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS acquisitions 
                                FROM reports.l8_report 
                                WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id::integer = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        proc AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.l8_report 
                                WHERE status_description = 'processed' AND l2_product IS NOT NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id::integer = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        ndld AS 
                            (SELECT acquisition_date, count(downloader_history_id) AS cnt 
                                FROM reports.l8_report 
                                WHERE status_description IN ('failed','aborted') AND l2_product IS NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id::integer = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        dld AS 
                            (SELECT acquisition_date, COUNT(downloader_history_id) AS cnt 
                                FROM reports.l8_report 
                                WHERE status_description IN ('downloaded', 'processing') AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id::integer = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        fproc AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.l8_report 
                                WHERE status_description = 'processed' AND l2_product IS NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id::integer = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        e AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.l8_report 
                                WHERE status_description LIKE 'processing_%failed' AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id::integer = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        cld AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.l8_report 
                                WHERE l8_report.clouds > 90 AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id::integer = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date)
                    SELECT 	c.cdate, 
                        COALESCE(ac.acquisitions, 0)::integer,
                        COALESCE(ndld.cnt, 0)::integer,
                        COALESCE(proc.cnt, 0)::integer, 
                        COALESCE(dld.cnt, 0)::integer,
                        COALESCE(fproc.cnt, 0)::integer,
                        COALESCE(e.cnt, 0)::integer,
                        COALESCE(cld.cnt, 0)::integer
                    FROM calendar c
                        LEFT JOIN ac ON ac.acquisition_date = c.cdate
                        LEFT JOIN ndld ON ndld.acquisition_date = c.cdate
                        LEFT JOIN proc ON proc.acquisition_date = c.cdate
                        LEFT JOIN dld ON dld.acquisition_date = c.cdate
                        LEFT JOIN fproc ON fproc.acquisition_date = c.cdate
                        LEFT JOIN e ON e.acquisition_date = c.cdate
                        LEFT JOIN cld ON cld.acquisition_date = c.cdate;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_l8_statistics(smallint, integer, date, date)
                  OWNER TO postgres;




                CREATE OR REPLACE FUNCTION reports.sp_reports_l8_statistics_orbit(
                    IN siteId smallint DEFAULT NULL::smallint,
                    IN orbitId integer DEFAULT NULL::integer,
                    IN fromDate date DEFAULT NULL::date,
                    IN toDate date DEFAULT NULL::date)
                  RETURNS TABLE(calendar_date date, acquisitions integer) AS
                $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-01-01')::date INTO startDate FROM l8_report;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-12-31') INTO endDate FROM l8_report;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    WITH 	calendar AS 
                            (SELECT date_trunc('day', dd)::date AS cdate 
                                FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
                        ac AS 
                            (SELECT acquisition_date, orbit_id, COUNT(DISTINCT downloader_history_id) AS acquisitions 
                                FROM reports.l8_report 
                                WHERE ($2 IS NULL OR orbit_id = $2::character varying) AND ($1 IS NULL OR site_id = $1) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date, orbit_id
                                ORDER BY acquisition_date)
                    SELECT 	c.cdate,
                        COALESCE(ac.acquisitions, 0)::integer
                    FROM calendar c
                        LEFT JOIN ac ON ac.acquisition_date = c.cdate;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_l8_statistics_orbit(smallint, integer, date, date)
                  OWNER TO postgres;




                CREATE OR REPLACE FUNCTION reports.sp_reports_orbits(
                    IN _satellite_id smallint,
                    IN _site_id smallint DEFAULT NULL::smallint)
                  RETURNS TABLE(orbit_id integer) AS
                $BODY$
                BEGIN
                    IF $1 = 3 THEN
                        RETURN QUERY
                        SELECT DISTINCT r.orbit_id::integer FROM reports.s1_report r WHERE ($2 IS NULL OR site_id = $2) ORDER BY r.orbit_id::integer;
                    ELSIF $1 = 1 THEN
                        RETURN QUERY
                        SELECT DISTINCT r.orbit_id::integer FROM reports.s2_report r WHERE ($2 IS NULL OR site_id = $2) ORDER BY r.orbit_id::integer;
                    ELSE
                        RETURN QUERY
                        SELECT DISTINCT r.orbit_id::integer FROM reports.l8_report r WHERE ($2 IS NULL OR site_id = $2) ORDER BY r.orbit_id::integer;
                    END IF;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_orbits(smallint, smallint)
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_s1_detail(
                    siteid smallint DEFAULT NULL::smallint,
                    orbitid integer DEFAULT NULL::integer,
                    fromdate date DEFAULT NULL::date,
                    todate date DEFAULT NULL::date)
                RETURNS TABLE(site_id smallint, downloader_history_id integer, orbit_id smallint, acquisition_date date, product_name character varying, status_description character varying, intersection_date date, intersected_product character varying, intersected_status_id smallint, intersection numeric, polarisation character varying, l2_product character varying, l2_coverage numeric, status_reason character varying) 
                    LANGUAGE 'plpgsql'
                    COST 100
                    STABLE 
                    ROWS 1000
                AS $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT MIN(r.acquisition_date) INTO startDate FROM reports.s1_report r;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT MAX(r.acquisition_date) INTO endDate FROM reports.s1_report r;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    SELECT r.site_id, r.downloader_history_id, r.orbit_id, r.acquisition_date, r.product_name, r.status_description, r.intersection_date, r.intersected_product,
                        r.intersected_status_id, r.intersection, r.polarisation, r.l2_product, r.l2_coverage, r.status_reason
                        FROM reports.s1_report r
                        WHERE ($1 IS NULL OR r.site_id = $1) AND ($2 IS NULL OR r.orbit_id = $2) AND r.acquisition_date BETWEEN startDate AND endDate
                    ORDER BY r.site_id, r.acquisition_date, r.product_name;
                END

                $BODY$

                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_s1_detail(smallint, integer, date, date)
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_s1_statistics(
                    IN siteId smallint DEFAULT NULL::smallint,
                    IN orbitId integer DEFAULT NULL::integer,
                    IN fromDate date DEFAULT NULL::date,
                    IN toDate date DEFAULT NULL::date)
                  RETURNS TABLE(calendar_date date, acquisitions integer, failed_to_download integer, pairs integer, 
                        processed integer, not_yet_processed integer, falsely_processed integer, no_intersections integer, errors integer) AS
                $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT MIN(acquisition_date) INTO startDate FROM reports.s1_report;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT MAX(acquisition_date) INTO endDate FROM reports.s1_report;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    WITH 	calendar AS 
                            (SELECT date_trunc('day', dd)::date AS cdate 
                                FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
                        ac AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS acquisitions 
                                FROM reports.s1_report 
                                WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        p AS 
                            (SELECT acquisition_date, COUNT(intersected_product) AS pairs 
                                FROM reports.s1_report 
                                WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        proc AS 
                            (SELECT acquisition_date, COUNT(intersected_product) AS cnt 
                                FROM reports.s1_report 
                                WHERE status_description = 'processed' AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        ndld AS 
                            (SELECT acquisition_date, count(downloader_history_id) AS cnt 
                                FROM reports.s1_report 
                                WHERE status_description IN ('failed','aborted') AND intersected_product IS NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        dld AS
                            (SELECT r.acquisition_date, COUNT(r.downloader_history_id) AS cnt
                                FROM reports.s1_report r
                                WHERE r.status_description IN ('downloaded', 'processing') AND r.intersected_product IS NOT NULL AND
                                    ($1 IS NULL OR r.site_id = $1) AND ($2 IS NULL OR r.orbit_id = $2) AND r.acquisition_date BETWEEN startDate AND endDate
                                     AND NOT EXISTS (SELECT s.downloader_history_id FROM reports.s1_report s
                                                    WHERE s.downloader_history_id = r.downloader_history_id AND r.l2_product LIKE '%COHE%')
                                GROUP BY acquisition_date
                                ORDER BY acquisition_date),
                        fproc AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.s1_report 
                                WHERE status_description = 'processed' AND intersected_product IS NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        ni AS 
                            (SELECT acquisition_date, count(downloader_history_id) AS cnt 
                                FROM reports.s1_report 
                                WHERE status_description = 'downloaded' AND intersected_product IS NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        e AS 
                            (SELECT acquisition_date, COUNT(intersected_product) AS cnt 
                                FROM reports.s1_report 
                                WHERE status_description LIKE 'processing_%failed' AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date)
                    SELECT 	c.cdate, 
                        COALESCE(ac.acquisitions, 0)::integer,
                        COALESCE(ndld.cnt, 0)::integer,
                        COALESCE(p.pairs, 0)::integer, 
                        COALESCE(proc.cnt, 0)::integer, 
                        COALESCE(dld.cnt, 0)::integer,
                        COALESCE(fproc.cnt, 0)::integer,
                        COALESCE(ni.cnt, 0)::integer,
                        COALESCE(e.cnt, 0)::integer
                    FROM calendar c
                        LEFT JOIN ac ON ac.acquisition_date = c.cdate
                        LEFT JOIN ndld ON ndld.acquisition_date = c.cdate
                        LEFT JOIN p ON p.acquisition_date = c.cdate
                        LEFT JOIN proc ON proc.acquisition_date = c.cdate
                        LEFT JOIN dld ON dld.acquisition_date = c.cdate
                        LEFT JOIN fproc ON fproc.acquisition_date = c.cdate
                        LEFT JOIN ni ON ni.acquisition_date = c.cdate
                        LEFT JOIN e ON e.acquisition_date = c.cdate;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_s1_statistics(smallint, integer, date, date)
                  OWNER TO postgres;




                CREATE OR REPLACE FUNCTION reports.sp_reports_s1_statistics_global(
                    IN _site_id smallint DEFAULT NULL::smallint,
                    IN _fromDate date DEFAULT NULL::date,
                    IN _toDate date DEFAULT NULL::date)
                  RETURNS TABLE(acquisitions integer, failed_to_download integer, pairs integer, 
                        processed integer, not_yet_processed integer, falsely_processed integer, no_intersections integer, errors integer) AS
                $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $2 IS NULL THEN
                        SELECT MIN(r.acquisition_date) INTO startDate FROM reports.s1_report r;
                    ELSE
                        SELECT _fromDate INTO startDate;
                    END IF;
                    IF $3 IS NULL THEN
                        SELECT MAX(r.acquisition_date) INTO endDate FROM reports.s1_report r;
                    ELSE
                        SELECT _toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    WITH 	calendar AS 
                            (SELECT date_trunc('day', dd)::date AS cdate 
                                FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
                        ac AS 
                            (SELECT r.acquisition_date, COUNT(DISTINCT downloader_history_id) AS acquisitions 
                                FROM reports.s1_report r
                                WHERE ($1 IS NULL OR r.site_id = $1) AND r.acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        p AS 
                            (SELECT r.acquisition_date, COUNT(intersected_product) AS pairs 
                                FROM reports.s1_report r
                                WHERE ($1 IS NULL OR r.site_id = $1) AND r.acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        proc AS 
                            (SELECT r.acquisition_date, COUNT(intersected_product) AS cnt 
                                FROM reports.s1_report r
                                WHERE r.status_description = 'processed' AND
                                    ($1 IS NULL OR r.site_id = $1) AND r.acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        ndld AS 
                            (SELECT r.acquisition_date, count(downloader_history_id) AS cnt 
                                FROM reports.s1_report r
                                WHERE r.status_description IN ('failed','aborted') AND r.intersected_product IS NULL AND
                                    ($1 IS NULL OR r.site_id = $1) AND r.acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        dld AS
                            (SELECT r.acquisition_date, COUNT(r.downloader_history_id) AS cnt
                                FROM reports.s1_report r
                                WHERE r.status_description IN ('downloaded', 'processing') AND r.intersected_product IS NOT NULL AND
                                    ($1 IS NULL OR r.site_id = $1) AND ($2 IS NULL OR r.orbit_id = $2) AND r.acquisition_date BETWEEN startDate AND endDate
                                     AND NOT EXISTS (SELECT s.downloader_history_id FROM reports.s1_report s
                                                    WHERE s.downloader_history_id = r.downloader_history_id AND r.l2_product LIKE '%COHE%')
                                GROUP BY acquisition_date
                                ORDER BY acquisition_date),
                        fproc AS 
                            (SELECT r.acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.s1_report r
                                WHERE r.status_description = 'processed' AND r.intersected_product IS NULL AND
                                    ($1 IS NULL OR r.site_id = $1) AND r.acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        ni AS 
                            (SELECT r.acquisition_date, count(downloader_history_id) AS cnt 
                                FROM reports.s1_report r
                                WHERE r.status_description = 'downloaded' AND r.intersected_product IS NULL AND
                                    ($1 IS NULL OR r.site_id = $1) AND r.acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        e AS 
                            (SELECT r.acquisition_date, COUNT(intersected_product) AS cnt 
                                FROM reports.s1_report r
                                WHERE r.status_description LIKE 'processing_%failed' AND
                                    ($1 IS NULL OR r.site_id = $1) AND r.acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date)
                    SELECT 	SUM(COALESCE(ac.acquisitions, 0))::integer,
                        SUM(COALESCE(ndld.cnt, 0))::integer,
                        SUM(COALESCE(p.pairs, 0))::integer,
                        SUM(COALESCE(proc.cnt, 0))::integer,
                        SUM(COALESCE(dld.cnt, 0))::integer,
                        SUM(COALESCE(fproc.cnt, 0))::integer,
                        SUM(COALESCE(ni.cnt, 0))::integer,
                        SUM(COALESCE(e.cnt, 0))::integer
                    FROM calendar c
                        LEFT JOIN ac ON ac.acquisition_date = c.cdate
                        LEFT JOIN ndld ON ndld.acquisition_date = c.cdate
                        LEFT JOIN p ON p.acquisition_date = c.cdate
                        LEFT JOIN proc ON proc.acquisition_date = c.cdate
                        LEFT JOIN dld ON dld.acquisition_date = c.cdate
                        LEFT JOIN fproc ON fproc.acquisition_date = c.cdate
                        LEFT JOIN ni ON ni.acquisition_date = c.cdate
                        LEFT JOIN e ON e.acquisition_date = c.cdate;
                END


                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_s1_statistics_global(smallint, date, date)
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_s1_statistics_orbit(
                    IN siteId smallint DEFAULT NULL::smallint,
                    IN orbitId integer DEFAULT NULL::integer,
                    IN fromDate date DEFAULT NULL::date,
                    IN toDate date DEFAULT NULL::date)
                  RETURNS TABLE(calendar_date date, orbit integer, acquisitions integer) AS
                $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-01-01')::date INTO startDate FROM reports.s1_report;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-12-31') INTO endDate FROM reports.s1_report;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    WITH 	calendar AS 
                            (SELECT date_trunc('day', dd)::date AS cdate 
                                FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
                        ac AS 
                            (SELECT acquisition_date, orbit_id, COUNT(DISTINCT downloader_history_id) AS acquisitions 
                                FROM reports.s1_report 
                                WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date, orbit_id
                                ORDER BY acquisition_date)
                    SELECT 	c.cdate,
                        COALESCE($1, ac.orbit_id, 0)::integer,
                        COALESCE(ac.acquisitions, 0)::integer
                    FROM calendar c
                        LEFT JOIN ac ON ac.acquisition_date = c.cdate;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_s1_statistics_orbit(smallint, integer, date, date)
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_s2_detail(
                    siteid smallint DEFAULT NULL::smallint,
                    orbitid integer DEFAULT NULL::integer,
                    fromdate date DEFAULT NULL::date,
                    todate date DEFAULT NULL::date)
                RETURNS TABLE(site_id smallint, downloader_history_id integer, orbit_id integer, acquisition_date date, product_name character varying, status_description character varying, status_reason character varying, l2_product character varying, clouds integer) 
                    LANGUAGE 'plpgsql'
                    COST 100
                    STABLE 
                    ROWS 1000
                AS $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT MIN(r.acquisition_date) INTO startDate FROM reports.s2_report r;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT MAX(r.acquisition_date) INTO endDate FROM reports.s2_report r;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    SELECT r.site_id, r.downloader_history_id, r.orbit_id::integer, r.acquisition_date, r.product_name, r.status_description, r.status_reason, r.l2_product, r.clouds
                        FROM reports.s2_report r
                        WHERE ($1 IS NULL OR r.site_id = $1) AND ($2 IS NULL OR r.orbit_id = $2) AND r.acquisition_date BETWEEN startDate AND endDate
                    ORDER BY r.site_id, r.acquisition_date, r.product_name;
                END


                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_s2_detail(smallint, integer, date, date)
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_s2_statistics(
                    IN siteId smallint DEFAULT NULL::smallint,
                    IN orbitId integer DEFAULT NULL::integer,
                    IN fromDate date DEFAULT NULL::date,
                    IN toDate date DEFAULT NULL::date)
                  RETURNS TABLE(calendar_date date, acquisitions integer, failed_to_download integer, processed integer, not_yet_processed integer, falsely_processed integer, errors integer, clouds integer) AS
                $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT MIN(acquisition_date) INTO startDate FROM reports.s2_report;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT MAX(acquisition_date) INTO endDate FROM reports.s2_report;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    WITH 	calendar AS 
                            (SELECT date_trunc('day', dd)::date AS cdate 
                                FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
                        ac AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS acquisitions 
                                FROM reports.s2_report 
                                WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        proc AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.s2_report 
                                WHERE status_description = 'processed' AND l2_product IS NOT NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        ndld AS 
                            (SELECT acquisition_date, count(downloader_history_id) AS cnt 
                                FROM reports.s2_report 
                                WHERE status_description IN ('failed','aborted') AND l2_product IS NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        dld AS 
                            (SELECT acquisition_date, COUNT(downloader_history_id) AS cnt 
                                FROM reports.s2_report 
                                WHERE status_description IN ('downloaded', 'processing') AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        fproc AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.s2_report 
                                WHERE status_description = 'processed' AND l2_product IS NULL AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        e AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.s2_report 
                                WHERE status_description LIKE 'processing_%failed' AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date),
                        cld AS 
                            (SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
                                FROM reports.s2_report 
                                WHERE s2_report.clouds > 90 AND
                                    ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date 
                                ORDER BY acquisition_date)
                    SELECT 	c.cdate, 
                        COALESCE(ac.acquisitions, 0)::integer,
                        COALESCE(ndld.cnt, 0)::integer,
                        COALESCE(proc.cnt, 0)::integer, 
                        COALESCE(dld.cnt, 0)::integer,
                        COALESCE(fproc.cnt, 0)::integer,
                        COALESCE(e.cnt, 0)::integer,
                        COALESCE(cld.cnt, 0)::integer
                    FROM calendar c
                        LEFT JOIN ac ON ac.acquisition_date = c.cdate
                        LEFT JOIN ndld ON ndld.acquisition_date = c.cdate
                        LEFT JOIN proc ON proc.acquisition_date = c.cdate
                        LEFT JOIN dld ON dld.acquisition_date = c.cdate
                        LEFT JOIN fproc ON fproc.acquisition_date = c.cdate
                        LEFT JOIN e ON e.acquisition_date = c.cdate
                        LEFT JOIN cld ON cld.acquisition_date = c.cdate;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_s2_statistics(smallint, integer, date, date)
                  OWNER TO postgres;



                CREATE OR REPLACE FUNCTION reports.sp_reports_s2_statistics_orbit(
                    IN siteId smallint DEFAULT NULL::smallint,
                    IN orbitId integer DEFAULT NULL::integer,
                    IN fromDate date DEFAULT NULL::date,
                    IN toDate date DEFAULT NULL::date)
                  RETURNS TABLE(calendar_date date, acquisitions integer) AS
                $BODY$
                DECLARE startDate date;
                DECLARE endDate date;
                BEGIN
                    IF $3 IS NULL THEN
                        SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-01-01')::date INTO startDate FROM s2_report;
                    ELSE
                        SELECT fromDate INTO startDate;
                    END IF;
                    IF $4 IS NULL THEN
                        SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-12-31') INTO endDate FROM s2_report;
                    ELSE
                        SELECT toDate INTO endDate;
                    END IF;
                    RETURN QUERY
                    WITH 	calendar AS 
                            (SELECT date_trunc('day', dd)::date AS cdate 
                                FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
                        ac AS 
                            (SELECT acquisition_date, orbit_id, COUNT(DISTINCT downloader_history_id) AS acquisitions 
                                FROM reports.s2_report 
                                WHERE ($2 IS NULL OR orbit_id = $2) AND ($1 IS NULL OR site_id = $1) AND acquisition_date BETWEEN startDate AND endDate
                                GROUP BY acquisition_date, orbit_id
                                ORDER BY acquisition_date)
                    SELECT 	c.cdate,
                        COALESCE(ac.acquisitions, 0)::integer
                    FROM calendar c
                        LEFT JOIN ac ON ac.acquisition_date = c.cdate;
                END

                $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION reports.sp_reports_s2_statistics_orbit(smallint, integer, date, date)
                  OWNER TO postgres;


  
            $str$;
            raise notice '%', _statement;
            execute _statement;

        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
