begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('1.1', '1.2')) then
            _statement := $str$     
                INSERT INTO config(key, site_id, value) VALUES ('downloader.s1.query.days.back', NULL, '0') on conflict DO nothing;
                INSERT INTO config(key, site_id, value) VALUES ('downloader.s2.query.days.back', NULL, '0') on conflict DO nothing;
                INSERT INTO config(key, site_id, value) VALUES ('downloader.l8.query.days.back', NULL, '0') on conflict DO nothing;
                
                CREATE TABLE IF NOT EXISTS fmask_history(
                    satellite_id smallint not null references satellite(id),
                    downloader_history_id int not null references downloader_history(id),
                    status_id int not null references l1_tile_status(id),
                    status_timestamp timestamp with time zone not null default now(),
                    retry_count int not null default 0,
                    failed_reason text,
                    primary key (downloader_history_id)
                );
            
                CREATE OR REPLACE FUNCTION sp_start_fmask_l1_tile_processing()
                returns table (
                    site_id int,
                    satellite_id smallint,
                    downloader_history_id int,
                    path text) as
                $$
                declare _satellite_id smallint;
                declare _downloader_history_id int;
                declare _path text;
                declare _site_id int;
                declare _product_date timestamp;
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    select fmask_history.satellite_id,
                           fmask_history.downloader_history_id
                    into _satellite_id,
                         _downloader_history_id
                    from fmask_history
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

                        update fmask_history
                        set status_id = 1, -- processing
                            status_timestamp = now()
                        where (fmask_history.downloader_history_id) = (_downloader_history_id);
                    else
                        select distinct
                            downloader_history.satellite_id,
                            downloader_history.id,
                            downloader_history.product_date,
                            downloader_history.full_path,
                            downloader_history.site_id
                        into _satellite_id,
                            _downloader_history_id,
                            _product_date,
                            _path,
                            _site_id
                        from downloader_history
                        inner join site on site.id = downloader_history.site_id
                        where not exists (
                            select *
                            from fmask_history
                            where (fmask_history.satellite_id) = (downloader_history.satellite_id)
                              and (status_id = 1 or -- processing
                                   retry_count < 3 and status_id = 2 -- failed
                              )
                              or (fmask_history.downloader_history_id) = (downloader_history.id)
                        ) and downloader_history.status_id in (2, 5, 7) -- downloaded, processing
                        and site.enabled
                        and downloader_history.satellite_id in (1, 2) -- sentinel2, landsat8
                        order by satellite_id, product_date
                        limit 1;

                        if found then
                            insert into fmask_history (
                                satellite_id,
                                downloader_history_id,
                                status_id
                            ) values (
                                _satellite_id,
                                _downloader_history_id,
                                1 -- processing
                            );
                        end if;
                    end if;

                    if _downloader_history_id is not null then
                        return query
                            select _site_id,
                                _satellite_id,
                                _downloader_history_id,
                                _path;
                    end if;
                end;
                $$ language plpgsql volatile;
                
                CREATE OR REPLACE FUNCTION sp_mark_fmask_l1_tile_done(
                    _downloader_history_id int
                )
                returns boolean
                as
                $$
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    update fmask_history
                    set status_id = 3, -- done
                        status_timestamp = now(),
                        failed_reason = null
                    where (downloader_history_id) = (_downloader_history_id);

                    return true;
                end;
                $$ language plpgsql volatile;
                
                CREATE OR REPLACE FUNCTION sp_mark_fmask_l1_tile_failed(
                    _downloader_history_id int,
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

                    update fmask_history
                    set status_id = 2, -- failed
                        status_timestamp = now(),
                        retry_count = case _should_retry
                            when true then retry_count + 1
                            else 3
                        end,
                        failed_reason = _reason
                    where (downloader_history_id) = (_downloader_history_id);

                    return true;
                end;
                $$ language plpgsql volatile;

                CREATE OR REPLACE FUNCTION sp_clear_pending_fmask_tiles()
                returns void
                as
                $$
                begin
                    delete
                    from fmask_history
                    where status_id = 1; -- processing
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;    


            _statement := 'update meta set version = ''1.2'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
