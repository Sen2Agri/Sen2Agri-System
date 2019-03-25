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
