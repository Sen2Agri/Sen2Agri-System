begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.8.2') then
            raise notice 'upgrading from 1.8.2 to 1.8.3';

            if not exists (SELECT column_name FROM information_schema.columns WHERE table_name='downloader_history' and column_name='footprint') then
                _statement := $str$
                ALTER TABLE downloader_history ADD COLUMN footprint geography null;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if exists (SELECT column_name, data_type FROM information_schema.columns WHERE table_name = 'downloader_history' AND column_name = 'tiles' and data_type = 'character varying') then
                _statement := $str$
                    ALTER TABLE downloader_history ADD COLUMN tmp text[];
                    UPDATE downloader_history SET tmp = string_to_array(tiles, ',') WHERE tiles IS NOT NULL;
                    ALTER TABLE downloader_history DROP COLUMN tiles;
                    ALTER TABLE downloader_history ADD COLUMN tiles text[];
                    UPDATE downloader_history SET tiles = tmp WHERE tmp IS NOT NULL;
                    ALTER TABLE downloader_history DROP COLUMN tmp;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config where key = 'executor.module.path.gdal_translate' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.gdal_translate', NULL, '/usr/bin/gdal_translate', '2018-08-30 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            if not exists (select * from config where key = 'executor.module.path.gdalbuildvrt' and site_id is null) then
                _statement := $str$
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.gdalbuildvrt', NULL, '/usr/bin/gdalbuildvrt', '2018-08-30 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_update_datasource(
                    _id smallint,
                    _scope smallint,
                    _enabled boolean,
                    _fetch_mode smallint,
                    _max_retries integer,
                    _retry integer,
                    _max_connections integer,
                    _download_path character varying,
                    _local_root character varying,
                    _username character varying,
                    _password character varying
                    )
                RETURNS void AS
                $BODY$
                    BEGIN
                    IF _id IS NOT NULL THEN
                        UPDATE datasource
                        SET enabled = _enabled,
                            scope = _scope,
                            fetch_mode = _fetch_mode,
                            max_retries = _max_retries,
                            retry_interval_minutes = _retry,
                            max_connections = _max_connections,
                            download_path = _download_path,
                            local_root = _local_root,
                            username = _username,
                            passwrd = _password
                        WHERE id = _id;
                    END IF;

                    END;
                $BODY$
                LANGUAGE plpgsql VOLATILE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_estimated_number_of_products(_siteid smallint DEFAULT NULL::smallint, _sentinel_1 boolean DEFAULT FALSE::boolean)
                RETURNS integer AS
                $BODY$
                DECLARE total_1 integer;
                DECLARE total_2 integer;
                DECLARE total_3 integer DEFAULT 0::integer;
                BEGIN
                    SELECT COALESCE(SUM(product_count),0)  into total_1 FROM downloader_count
                    WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,min(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid ) AND product_count >=0 AND satellite_id=1  GROUP BY site_id );

                    SELECT COALESCE(SUM(product_count),0)  into total_2 FROM downloader_count
                    WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,min(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid) AND product_count >=0 AND satellite_id=2 GROUP BY site_id );

                    IF(_sentinel_1) THEN
                        SELECT COALESCE(SUM(product_count),0)  into total_3 FROM downloader_count
                        WHERE (_siteid IS NULL OR site_id = _siteid) AND (site_id,last_updated) IN (SELECT site_id,min(last_updated) FROM downloader_count WHERE (_siteid IS NULL OR site_id = _siteid) AND product_count >=0 AND satellite_id=3 GROUP BY site_id );
                    END IF;

                    RETURN (total_1 + total_2 + total_3) as nr;
                END
                $BODY$
                LANGUAGE plpgsql STABLE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                drop function if exists sp_get_dashboard_processor_scheduled_task(smallint);
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_dashboard_processor_scheduled_task(IN _processor_id smallint)
                RETURNS TABLE(
                    id smallint,
                    name character varying,
                    site_id smallint,
                    site_name character varying,
                    season_name text,
                    repeat_type smallint,
                    first_run_time character varying,
                    repeat_after_days smallint,
                    repeat_on_month_day smallint,
                    processor_params character varying) AS
                $BODY$
                BEGIN
                RETURN QUERY
                    SELECT st.id,
                            st.name,
                            site.id,
                            site.name,
                            season.name,
                            st.repeat_type,
                            st.first_run_time,
                            st.repeat_after_days,
                            st.repeat_on_month_day,
                            st.processor_params
                        FROM scheduled_task AS st
                        INNER JOIN site on site.id = st.site_id
                        LEFT OUTER JOIN season ON season.id = st.season_id
                        WHERE st.processor_id = _processor_id
                        ORDER BY st.id;


                END
                $BODY$
                LANGUAGE plpgsql;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                drop function if exists sp_get_dashboard_downloader_history(smallint);
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_dashboard_downloader_history(IN _siteid smallint DEFAULT NULL::smallint)
                RETURNS TABLE(id integer, nr_downloads bigint, nr_downloads_percentage numeric) AS
                $BODY$
                DECLARE total numeric;

                                BEGIN

                        Select count(dh.id) into total from downloader_history dh WHERE status_id IN (1, 2, 3, 4, 5, 6, 7 ,41) AND (_siteid IS NULL OR site_id = _siteid);

                                IF total<>'0' THEN
                            RETURN QUERY

                                SELECT  1, count(status_id),CASE WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
                                ELSE 0 END

                                FROM downloader_history
                                WHERE status_id  = 1 AND ( $1 IS NULL OR site_id = _siteid)
                            UNION
                                SELECT  2, count(status_id),CASE  WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
                                ELSE 0 END
                                FROM downloader_history
                                WHERE status_id IN (2, 5, 6, 7) AND ( $1 IS NULL OR site_id = _siteid)
                            UNION
                                SELECT  3, count(status_id),CASE  WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
                                ELSE 0 END
                                FROM downloader_history
                                WHERE status_id IN (4,41) AND ( $1 IS NULL OR site_id = _siteid)
                                UNION
                                SELECT  4, count(status_id),CASE  WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
                                ELSE 0 END
                                FROM downloader_history
                                WHERE status_id IN (3) AND ( $1 IS NULL OR site_id = _siteid)
                            ORDER BY 1;

                        END IF;
                                END
                                $BODY$
                LANGUAGE plpgsql STABLE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                drop function sp_get_dashboard_products(smallint, integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[]);
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
                    _site_id integer[] DEFAULT NULL::integer[],
                    _product_type_id integer[] DEFAULT NULL::integer[],
                    _season_id smallint DEFAULT NULL::smallint,
                    _satellit_id integer[] DEFAULT NULL::integer[],
                    _since_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _until_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _tiles character varying[] DEFAULT NULL::character varying[])
                RETURNS SETOF json AS
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
                                data(id, satellite_id, product, product_type_id, product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
                                SELECT
                                P.id,
                                P.satellite_id,
                                P.name,
                                PT.id,
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
                                AND EXISTS (
                                    SELECT * FROM season WHERE season.site_id =P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date
                            $sql$;
                            IF $3 IS NOT NULL THEN
                            q := q || $sql$
                                AND season.id=$3
                                $sql$;
                            END IF;

                            q := q || $sql$
                            )
                            $sql$;

                            IF $1 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.site_id = ANY($1)
                            $sql$;
                            END IF;

                            IF $2 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.product_type_id= ANY($2)

                                $sql$;
                            END IF;

                        IF $5 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp >= to_timestamp(cast($5 as TEXT),'YYYY-MM-DD HH24:MI:SS')
                            $sql$;
                        END IF;

                        IF $6 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp <= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day'
                            $sql$;
                        END IF;

                        IF $7 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.tiles <@$7 AND P.tiles!='{}'
                            $sql$;
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
                            USING _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles;
                        END
                        $BODY$
                LANGUAGE plpgsql STABLE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_dashboard_products_nodes(
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
                                ),
                        $sql$;

                        IF $8 IS TRUE THEN
                                q := q || $sql$
                            site_names(id, name, geog, row) AS (
                                select id, name, st_astext(geog), row_number() over (order by name)
                                from site
                                ),
                            data(id, product, footprint, site_coord, product_type_id, satellite_id, is_raster ) AS (

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
                                    SELECT * FROM season WHERE season.site_id = P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date
                            $sql$;
                            IF $3 IS NOT NULL THEN
                            q := q || $sql$
                                AND season.id=$3
                                $sql$;
                            END IF;

                            q := q || $sql$
                            )
                            $sql$;
                            raise notice '%', _site_id;raise notice '%', _product_type_id;raise notice '%', _satellit_id;
                            IF $1 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.site_id = ANY($1)

                            $sql$;
                            END IF;

                            IF $2 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.product_type_id= ANY($2)

                                $sql$;
                            END IF;

                        IF $5 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp >= to_timestamp(cast($5 as TEXT),'YYYY-MM-DD HH24:MI:SS')
                            $sql$;
                        END IF;

                        IF $6 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp <= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day'
                            $sql$;
                        END IF;

                        IF $7 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.tiles <@$7 AND P.tiles!='{}'
                            $sql$;
                        END IF;

                        /*IF $4 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.satellite_id= ANY($4)

                            $sql$;
                        END IF;*/


                        q := q || $sql$
                            ORDER BY S.row, PT.row, P.name
                            )
                        --         select * from data;
                            SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
                            $sql$;

                            raise notice '%', q;

                            RETURN QUERY
                            EXECUTE q
                            USING _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles, _get_nodes;
                        END
                        $BODY$
                LANGUAGE plpgsql STABLE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                create table if not exists l1_tile_status(
                    id smallint not null primary key,
                    description text not null
                );
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                create table if not exists l1_tile_history(
                    satellite_id smallint not null references satellite(id),
                    orbit_id int not null,
                    tile_id text not null,
                    downloader_history_id int not null references downloader_history(id),
                    status_id int not null references l1_tile_status(id),
                    status_timestamp timestamp with time zone not null default now(),
                    retry_count int not null default 0,
                    failed_reason text,
                    cloud_coverage int,
                    snow_coverage int,
                    primary key (downloader_history_id, tile_id)
                );
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                create or replace function sp_update_l1_tile_status(
                    _downloader_history_id int
                )
                returns boolean
                as
                $$
                begin
                    if not exists(
                        select unnest(tiles)
                        from downloader_history
                        where id = _downloader_history_id
                        except all
                        select tile_id
                        from l1_tile_history
                        where downloader_history_id = _downloader_history_id
                        and (status_id = 3 -- done
                            or retry_count = 3 and status_id = 2) -- failed
                    ) then
                        if exists(
                            select *
                            from l1_tile_history
                            where downloader_history_id = _downloader_history_id
                            and status_id = 3 -- done
                        ) then
                            update downloader_history
                            set status_id = 5 -- processed
                            where id = _downloader_history_id;
                        else
                            update downloader_history
                            set status_id = 6 -- processing_failed
                            where id = _downloader_history_id;
                        end if;
                        return true;
                    else
                        return false;
                    end if;
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                create or replace function sp_mark_l1_tile_done(
                    _downloader_history_id int,
                    _tile_id text,
                    _cloud_coverage int,
                    _snow_coverage int
                )
                returns boolean
                as
                $$
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    update l1_tile_history
                    set status_id = 3, -- done
                        status_timestamp = now(),
                        failed_reason = null,
                        cloud_coverage = _cloud_coverage,
                        snow_coverage = _snow_coverage
                    where (downloader_history_id, tile_id) = (_downloader_history_id, _tile_id);

                    return sp_update_l1_tile_status(_downloader_history_id);
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                create or replace function sp_mark_l1_tile_failed(
                    _downloader_history_id int,
                    _tile_id text,
                    _reason text,
                    _should_retry boolean
                )
                returns boolean
                as
                $$
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    update l1_tile_history
                    set status_id = 2, -- failed
                        status_timestamp = now(),
                        retry_count = case _should_retry
                            when true then retry_count + 1
                            else 3
                        end,
                        failed_reason = _reason,
                        cloud_coverage = _cloud_coverage,
                        snow_coverage = _snow_coverage
                    where (downloader_history_id, tile_id) = (_downloader_history_id, _tile_id);

                    return sp_update_l1_tile_status(_downloader_history_id);
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                create or replace function sp_start_l1_tile_processing()
                returns table (
                    site_id int,
                    satellite_id smallint,
                    orbit_id int,
                    tile_id text,
                    downloader_history_id int,
                    path text,
                    prev_l2a_path text
                ) as
                $$
                declare _satellite_id smallint;
                declare _orbit_id int;
                declare _tile_id text;
                declare _downloader_history_id int;
                declare _path text;
                declare _prev_l2a_path text;
                declare _site_id int;
                declare _product_date timestamp;
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    select l1_tile_history.satellite_id,
                        l1_tile_history.orbit_id,
                        l1_tile_history.tile_id,
                        l1_tile_history.downloader_history_id
                    into _satellite_id,
                        _orbit_id,
                        _tile_id,
                        _downloader_history_id
                    from l1_tile_history
                    where status_id = 2 -- failed
                    and retry_count < 3
                    and status_timestamp < now() - interval '1 day'
                    order by status_timestamp
                    limit 1;

                    if found then
                        select downloader_history.product_date,
                            downloader_history.full_path,
                            downloader_history.site_id
                        into _product_date,
                            _path,
                            _site_id
                        from downloader_history
                        where id = _downloader_history_id;

                        update l1_tile_history
                        set status_id = 1, -- processing
                            status_timestamp = now()
                        where (l1_tile_history.satellite_id, l1_tile_history.orbit_id, l1_tile_history.tile_id) = (_satellite_id, _orbit_id, _tile_id);
                    else
                        select distinct
                            downloader_history.satellite_id,
                            downloader_history.orbit_id,
                            tile_ids.tile_id,
                            downloader_history.id,
                            downloader_history.product_date,
                            downloader_history.full_path,
                            downloader_history.site_id
                        into _satellite_id,
                            _orbit_id,
                            _tile_id,
                            _downloader_history_id,
                            _product_date,
                            _path,
                            _site_id
                        from downloader_history
                        cross join lateral (
                                select unnest(tiles) as tile_id
                            ) tile_ids
                        inner join site on site.id = downloader_history.site_id
                        where not exists (
                            select *
                            from l1_tile_history
                            where (l1_tile_history.satellite_id,
                                l1_tile_history.orbit_id,
                                l1_tile_history.tile_id) =
                                (downloader_history.satellite_id,
                                downloader_history.orbit_id,
                                tile_ids.tile_id)
                            and (status_id = 1 or -- processing
                                retry_count < 3 and status_id = 2 -- failed
                            )
                            or (l1_tile_history.downloader_history_id, l1_tile_history.tile_id) = (downloader_history.id, tile_ids.tile_id)
                        ) and downloader_history.status_id in (2, 7) -- downloaded, processing
                        and site.enabled
                        and downloader_history.satellite_id in (1, 2) -- sentinel2, landsat8
                        order by satellite_id,
                                orbit_id,
                                tile_id,
                                product_date
                        limit 1;

                        if found then
                            insert into l1_tile_history (
                                satellite_id,
                                orbit_id,
                                tile_id,
                                downloader_history_id,
                                status_id
                            ) values (
                                _satellite_id,
                                _orbit_id,
                                _tile_id,
                                _downloader_history_id,
                                1 -- processing
                            );

                            update downloader_history
                            set status_id = 7 -- processing
                            where id = _downloader_history_id;
                        end if;
                    end if;

                    if _downloader_history_id is not null then
                        select product.full_path
                        into _prev_l2a_path
                        from product
                        where product.site_id = _site_id
                        and product.product_type_id = 1 -- l2a
                        and product.satellite_id = _satellite_id
                        and product.created_timestamp < _product_date
                        and product.tiles :: text[] @> array[_tile_id]
                        and (product.satellite_id <> 1 -- sentinel2
                            or product.orbit_id = _orbit_id)
                        order by created_timestamp desc
                        limit 1;

                        return query
                            select _site_id,
                                _satellite_id,
                                _orbit_id,
                                _tile_id,
                                _downloader_history_id,
                                _path,
                                _prev_l2a_path;
                    end if;
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            if not exists (
                select *
                from pg_attribute
                where attrelid = 'downloader_history' :: regclass
                and attnum > 0
                and attname = 'tiles'
                and not attisdropped
            ) then
                _statement := $str$
                    alter table downloader_history add column tiles text[];
                $str$;
                raise notice '%', _statement;
                execute _statement;
            elsif (
                select format_type(atttypid, atttypmod)
                from pg_attribute
                where attrelid = 'downloader_history' :: regclass
                and attnum > 0
                and attname = 'tiles'
                and not attisdropped
            ) in ('character varying', 'text') then
                _statement := $str$
                    alter table downloader_history add column tiles_migrate text[];
                    update downloader_history
                    set tiles_migrate = regexp_split_to_array(tiles, ',');
                    alter table downloader_history drop column tiles;
                    alter table downloader_history add column tiles text[];
                    update downloader_history
                    set tiles = tiles_migrate;
                    alter table downloader_history drop column tiles_migrate;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;

			if not exists (
				select *
				from pg_attribute
				where attrelid = 'user' :: regclass
				and attnum > 0
				and attname = 'site_id'
				and not attisdropped
			) then
				_statement := $str$
					alter table 'user' add column site_id integer[];
					$str$;
                raise notice '%', _statement;
                execute _statement;
			elsif (
				select format_type(atttypid, atttypmod)
				from pg_attribute
				where attrelid = 'user' :: regclass
				and attnum > 0
				and attname = 'site_id'
				and not attisdropped
			) in ('smallint','integer') then
				_statement := $str$
					alter table "user" drop constraint user_fk1;
					alter table "user" alter column site_id drop default;
					alter table "user" alter column site_id type integer[]
					using case when site_id is null then null else array[site_id]::integer[] end;
					alter table "user" alter column password drop not null;
				$str$;
                raise notice '%', _statement;
                execute _statement;
			end if;

			_statement := $str$
				CREATE OR REPLACE FUNCTION sp_get_all_users()
				RETURNS TABLE(user_id smallint, user_name character varying, email character varying, role_id smallint, role_name character varying, site_id integer[], site_name text) AS
				$BODY$
				BEGIN
				RETURN QUERY
					SELECT u.id as user_id,
						u.login as user_name,
						u. email,
						u.role_id,
						r.name as role_name,
						u.site_id,
						(SELECT array_to_string(array_agg(name),', ') from site where id = ANY(u.site_id::int[]) ) as site_name
					FROM "user" u
					INNER JOIN role r ON u.role_id = r.id;

				END;
				$BODY$
				LANGUAGE plpgsql VOLATILE;
				$str$;
			raise notice '%', _statement;
			execute _statement;

            if not exists (
                select *
                from pg_attribute
                where attrelid = 'downloader_history' :: regclass
                and attnum > 0
                and attname = 'footprint'
                and not attisdropped
            ) then
                _statement := $str$
                    alter table downloader_history add column footprint geography;
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;


			if not exists (
				select *
				from pg_attribute
				where attrelid = 'config_metadata' :: regclass
				and attnum > 0
				and attname = 'is_site_visible'
				and not attisdropped
			) then
				_statement := $str$
					alter table config_metadata add column is_site_visible boolean NOT NULL DEFAULT false;
					$str$;
				raise notice '%', _statement;
				execute _statement;
			end if;


			if not exists (
				select *
				from pg_attribute
				where attrelid = 'config_metadata' :: regclass
				and attnum > 0
				and attname = 'label'
				and not attisdropped
			) then
				_statement := $str$
					alter table config_metadata add column label character varying;
					$str$;
				raise notice '%', _statement;
				execute _statement;
			end if;

			if not exists (
				select *
				from pg_attribute
				where attrelid = 'config_metadata' :: regclass
				and attnum > 0
				and attname = 'values'
				and not attisdropped
			) then
				_statement := $str$
					alter table config_metadata add column values json;
					$str$;
				raise notice '%', _statement;
				execute _statement;
			end if;

			_statement := $str$
				UPDATE config_metadata
				SET is_site_visible = true
				WHERE
					key in ('processor.l3a.weight.cloud.coarseresolution'
					,'processor.l3a.weight.total.weightdatemin'
					,'processor.l3a.weight.aot.maxaot'
					,'processor.l3a.weight.aot.minweight'
					,'processor.l3a.weight.aot.maxweight'
					,'processor.l3a.weight.cloud.sigmasmall'
					,'processor.l3a.weight.cloud.sigmalarge'
					,'processor.l3a.half_synthesis'
					,'processor.l3b.generate_models'
					,'processor.l4a.random_seed'
					,'processor.l4a.window'
					,'processor.l4a.nbcomp'
					,'processor.l4a.range-radius'
					,'processor.l4a.erode-radius'
					,'processor.l4a.mahalanobis-alpha'
					,'processor.l4a.segmentation-spatial-radius'
					,'processor.l4a.segmentation-minsize'
					,'processor.l4a.classifier.rf.nbtrees'
					,'processor.l4a.classifier.rf.max'
					,'processor.l4a.classifier.rf.min'
					,'processor.l4a.min-area'
					,'processor.l4b.random_seed'
					,'processor.l4b.classifier.rf.nbtrees'
					,'processor.l4b.classifier.rf.max'
					,'processor.l4b.classifier.rf.min');
				$str$;
				raise notice '%', _statement;
				execute _statement;

			_statement := $str$
				UPDATE config_metadata
				SET is_advanced = true
				WHERE
					key in ('processor.l3a.weight.cloud.coarseresolution'
					,'processor.l3a.weight.total.weightdatemin'
					,'processor.l3a.weight.aot.maxaot'
					,'processor.l3a.weight.aot.minweight'
					,'processor.l3a.weight.aot.maxweight'
					,'processor.l3a.weight.cloud.sigmasmall'
					,'processor.l3a.weight.cloud.sigmalarge'
					,'processor.l3b.generate_models'
					,'processor.l4a.random_seed'
					,'processor.l4a.window'
					,'processor.l4a.nbcomp'
					,'processor.l4a.range-radius'
					,'processor.l4a.erode-radius'
					,'processor.l4a.mahalanobis-alpha'
					,'processor.l4a.segmentation-spatial-radius'
					,'processor.l4a.segmentation-minsize'
					,'processor.l4a.classifier.rf.nbtrees'
					,'processor.l4a.classifier.rf.max'
					,'processor.l4a.classifier.rf.min'
					,'processor.l4a.min-area'
					,'processor.l4b.random_seed'
					,'processor.l4b.classifier.rf.nbtrees'
					,'processor.l4b.classifier.rf.max'
					,'processor.l4b.classifier.rf.min');
				$str$;
				raise notice '%', _statement;
				execute _statement;

			_statement := $str$
				UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"1"}'::json, label ='Generate models' WHERE key ='processor.l3b.generate_models';
				$str$;
				raise notice '%', _statement;
				execute _statement;

			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Maximum value of the linear range for weights w.r.t. AOT' WHERE key ='processor.l3a.weight.aot.maxaot';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Minimum weight depending on AOT'  WHERE key ='processor.l3a.weight.aot.minweight';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Maximum weight depending on AOT'  WHERE key ='processor.l3a.weight.aot.maxweight';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Standard deviation of gaussian filter for distance to small clouds'  WHERE key ='processor.l3a.weight.cloud.sigmasmall';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Standard deviation of gaussian filter for distance to large clouds'  WHERE key ='processor.l3a.weight.cloud.sigmalarge';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Coarse resolution for quicker convolution'  WHERE key ='processor.l3a.weight.cloud.coarseresolution';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Minimum weight at edge of the synthesis time window'  WHERE key ='processor.l3a.weight.total.weightdatemin';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Half synthesis'  WHERE key ='processor.l3a.half_synthesis';
			UPDATE config_metadata SET values ='{}'::json,is_site_visible = false  WHERE key ='processor.l3a.synth_date_sched_offset';

			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"1"}'::json, label ='Generate models' WHERE key ='processor.l3b.generate_models';

			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Random seed' WHERE key ='processor.l4a.random_seed';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Window records' WHERE key ='processor.l4a.window';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Number of components' WHERE key ='processor.l4a.nbcomp';
			UPDATE config_metadata SET values ='{"min":"0","step":"0.01","max":""}'::json, label ='Range radius' WHERE key ='processor.l4a.range-radius';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Erosion radius' WHERE key ='processor.l4a.erode-radius';
			UPDATE config_metadata SET values ='{"min":"0","step":"0.01","max":""}'::json, label ='Erosion radius' WHERE key ='processor.l4a.mahalanobis-alpha';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"segmentation"}'::json, label ='Spatial radius' WHERE key ='processor.l4a.segmentation-spatial-radius';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"segmentation"}'::json, label ='Minimum size of a region' WHERE key ='processor.l4a.segmentation-minsize';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"classifier"}'::json, label ='Training trees' WHERE key ='processor.l4a.classifier.rf.nbtrees';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"classifier"}'::json, label ='Max depth' WHERE key ='processor.l4a.classifier.rf.max';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"classifier"}'::json, label ='Minimum number of samples'WHERE key ='processor.l4a.classifier.rf.min';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"classifier"}'::json, label ='The minium number of pixels' WHERE key ='processor.l4a.min-area';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json,is_advanced = false, is_site_visible = true, label ='Ratio' WHERE key ='processor.l4a.sample-ratio';

			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, label ='Random seed'  WHERE key ='processor.l4b.random_seed';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"classifier"}'::json, label ='Training trees'  WHERE key ='processor.l4b.classifier.rf.nbtrees';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"classifier"}'::json, label ='Random Forest classifier max depth'  WHERE key ='processor.l4b.classifier.rf.max';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":"","type":"classifier"}'::json, label ='Minimum number of samples'  WHERE key ='processor.l4b.classifier.rf.min';
			UPDATE config_metadata SET values ='{"name":"cropMask"}', is_site_visible = true, is_advanced = false, label ='Crop masks'  WHERE key ='processor.l4b.crop-mask';
			UPDATE config_metadata SET values ='{"min":"0","step":"1","max":""}'::json, is_site_visible = true, is_advanced = false, label ='Ratio'  WHERE key ='processor.l4b.sample-ratio';

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_set_user_password(
                    IN user_name character varying,
                    IN email character varying,
                    IN pwd text
                    )RETURNS character varying AS
                    $BODY$
                    DECLARE user_id smallint;

                    BEGIN
                        SELECT id into user_id FROM "user" WHERE "user".login = $1 AND "user".email = $2;

                        IF user_id IS NOT NULL THEN
                            IF char_length(trim(pwd))>0 THEN

                                UPDATE "user"
                                     SET password = crypt($3, gen_salt('md5'))
                                     WHERE id = user_id ;--AND password = crypt(user_pwd, password);
                                RETURN 1;
                            ELSE
                                RETURN 0;
                            END IF;
                        ELSE RETURN 2;
                        END IF;
                END;
                $BODY$
                LANGUAGE plpgsql VOLATILE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                drop function sp_authenticate(character varying, text);
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_authenticate(IN usr character varying, IN pwd text)
                  RETURNS TABLE(user_id smallint, site_id integer[], role_id smallint, role_name character varying) AS
                $BODY$SELECT u.id, u.site_id, r.id, r.name
                  FROM "user" u
                  INNER JOIN role r ON u.role_id=r.id
                  WHERE login = $1 AND crypt($2, password) = password
                $BODY$
                  LANGUAGE sql VOLATILE;
            $str$;
            raise notice '%', _statement;
            execute _statement;

			if not exists (
				select *
				from pg_attribute
				where attrelid = 'product_type' :: regclass
				and attnum > 0
				and attname = 'is_raster'
				and not attisdropped
			) then
				_statement := $str$
					alter table product_type add column is_raster boolean NOT NULL DEFAULT TRUE;
					$str$;
				raise notice '%', _statement;
				execute _statement;
			end if;


            _statement := 'update meta set version = ''1.8.3'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
