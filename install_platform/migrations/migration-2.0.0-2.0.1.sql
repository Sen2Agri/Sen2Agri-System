begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '2.0') then
            raise notice 'upgrading from 2.0 to 2.0.1';

            raise notice 'patching 2.0';

            _statement := $str$
                DROP FUNCTION IF EXISTS sp_insert_product(
                    _product_type_id smallint,
                    _processor_id smallint,
                    _satellite_id integer,
                    _site_id smallint,
                    _job_id integer,
                    _full_path character varying,
                    _created_timestamp timestamp with time zone,
                    _name character varying,
                    _quicklook_image character varying,
                    _footprint geography,
                    _orbit_id integer,
                    _tiles json);
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            if not exists (select * from l1_tile_status) then
                _statement := $str$
                    INSERT INTO l1_tile_status VALUES (1, 'processing'), (2, 'failed'), (3, 'done');
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;                

             _statement := $str$
                create or replace function sp_clear_pending_l1_tiles()
                returns void
                as
                $$
                begin
                    delete
                    from l1_tile_history
                    where status_id = 1; -- processing

                    update downloader_history
                    set status_id = 2 -- downloaded
                    where status_id = 7 -- processing
                      and not exists (
                        select *
                        from l1_tile_history
                        where status_id = 1 -- processing
                    );
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;  


            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_insert_product(
                    _product_type_id smallint,
                    _processor_id smallint,
                    _satellite_id integer,
                    _site_id smallint,
                    _job_id integer,
                    _full_path character varying,
                    _created_timestamp timestamp with time zone,
                    _name character varying,
                    _quicklook_image character varying,
                    _footprint geography,
                    _orbit_id integer,
                    _tiles json,
                    _orbit_type_id smallint DEFAULT NULL::smallint,
                    _downloader_history_id integer DEFAULT NULL::integer)
                  RETURNS integer AS
                $BODY$
                DECLARE return_id product.id%TYPE;
                BEGIN
                    UPDATE product
                    SET job_id = _job_id,
                        full_path = _full_path,
                        created_timestamp = _created_timestamp,
                        quicklook_image = _quicklook_image,
                        footprint = (SELECT '(' || string_agg(REPLACE(replace(ST_AsText(geom) :: text, 'POINT', ''), ' ', ','), ',') || ')'
                                     from ST_DumpPoints(ST_Envelope(_footprint :: geometry))
                                     WHERE path[2] IN (1, 3)) :: POLYGON,
                        geog = _footprint,
                        tiles = array(select tile :: character varying from json_array_elements_text(_tiles) tile),
                        is_archived = FALSE
                    WHERE product_type_id = _product_type_id
                      AND processor_id = _processor_id
                      AND satellite_id = _satellite_id
                      AND site_id = _site_id
                      AND COALESCE(orbit_id, 0) = COALESCE(_orbit_id, 0)
                      AND "name" = _name
                    RETURNING id INTO return_id;

                    IF NOT FOUND THEN
                        INSERT INTO product(
                            product_type_id,
                            processor_id,
                            satellite_id,
                            job_id,
                            site_id,
                            full_path,
                            created_timestamp,
                            "name",
                            quicklook_image,
                            footprint,
                            geog,
                            orbit_id,
                            tiles,
                            orbit_type_id,
                            downloader_history_id
                        )
                        VALUES (
                            _product_type_id,
                            _processor_id,
                            _satellite_id,
                            _job_id,
                            _site_id,
                            _full_path,
                            COALESCE(_created_timestamp, now()),
                            _name,
                            _quicklook_image,
                            (SELECT '(' || string_agg(REPLACE(replace(ST_AsText(geom) :: text, 'POINT', ''), ' ', ','), ',') || ')'
                             from ST_DumpPoints(ST_Envelope(_footprint :: geometry))
                             WHERE path[2] IN (1, 3)) :: POLYGON,
                             _footprint,
                             _orbit_id,
                            array(select tile :: character varying from json_array_elements_text(_tiles) tile),
                            _orbit_type_id,
                            _downloader_history_id
                        )
                        RETURNING id INTO return_id;
                        
                        INSERT INTO event(
                            type_id,
                            data,
                            submitted_timestamp)
                            VALUES (
                            3, -- "ProductAvailable"
                            ('{"product_id":' || return_id || '}') :: json,
                            now()
                        );
                    END IF;

                    RETURN return_id;
                END;
                $BODY$
                  LANGUAGE plpgsql VOLATILE;
            $str$;
            raise notice '%', _statement;
            execute _statement;  

             _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_product_by_id(IN _id integer)
                  RETURNS TABLE(product_id integer, product_type_id smallint, processor_id smallint, site_id smallint, full_path character varying, created_timestamp timestamp with time zone, inserted_timestamp timestamp with time zone) AS
                $BODY$
                                BEGIN
                                    RETURN QUERY SELECT product.id AS product_id, product.product_type_id, product.processor_id, product.site_id, product.full_path, product.created_timestamp, product.inserted_timestamp 
                                    FROM product
                                    WHERE product.id = _id;
                                END;
                                $BODY$
                  LANGUAGE plpgsql VOLATILE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION sp_get_product_by_id(integer)
                  OWNER TO admin;
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
                        where (l1_tile_history.downloader_history_id, l1_tile_history.tile_id) = (_downloader_history_id, _tile_id);
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


            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_product_by_id(_id integer)
                    RETURNS TABLE(product_id integer, product_type_id smallint, processor_id smallint, site_id smallint, full_path character varying, created_timestamp timestamp with time zone, inserted_timestamp timestamp with time zone) AS
                $BODY$
                BEGIN
                    RETURN QUERY SELECT product.product_type_id AS product_id, product.product_type_id, product.processor_id, product.site_id, product.full_path, product.created_timestamp, product.inserted_timestamp 
                    FROM product
                    WHERE product.product_type_id = _id;
                END;
                $BODY$
                    LANGUAGE plpgsql VOLATILE
                    COST 100
                    ROWS 1000;
                ALTER FUNCTION sp_get_product_by_id(integer)
                    OWNER TO admin;
            $str$;
            raise notice '%', _statement;
            execute _statement;   
            
            
            if not exists (select * from config where key = 'processor.l3b_lai.sub_products') then
                _statement := $str$
                    INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b_lai.sub_products', NULL, 'L3B,L3C,L3D', '2019-04-12 14:56:57.501918+02');
                $str$;
                raise notice '%', _statement;
                execute _statement;   
            end if;    
            _statement := $str$
                create or replace function sp_clear_pending_l1_tiles()
                    returns void
                    as
                    $$
                    begin
                        delete
                        from l1_tile_history
                        where status_id = 1; -- processing

                        update downloader_history
                        set status_id = 2 -- downloaded
                        where status_id = 7 -- processing
                          and not exists (
                            select *
                            from l1_tile_history
                            where status_id = 1 -- processing
                        );
                    end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            UPDATE config SET value = 8082 WHERE key = 'http-listener.listen-port';
            
            _statement := 'update meta set version = ''2.0.1'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
