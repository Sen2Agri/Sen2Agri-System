create or replace function sp_start_fmask_l1_tile_processing()
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
