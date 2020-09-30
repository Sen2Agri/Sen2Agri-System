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

    create temporary table if not exists site_config(
        key,
        site_id,
        value
    ) as
    select
        keys.key,
        site.id,
        config.value
    from site
    cross join (
        values
            ('processor.l2a.s2.implementation'),
            ('processor.l2a.optical.retry-interval'),
            ('processor.l2a.optical.max-retries'),
            ('processor.l2a.optical.num-workers'),
            ('s2.enabled'),
            ('l8.enabled')
    ) as keys(key)
    cross join lateral (
        select
            coalesce((
                select value
                from config
                where key = keys.key
                and config.site_id = site.id
            ), (
                select value
                from config
                where key = keys.key
                and config.site_id is null
            )) as value
    ) config;

    select l1_tile_history.satellite_id,
           l1_tile_history.orbit_id,
           l1_tile_history.tile_id,
           l1_tile_history.downloader_history_id
    into _satellite_id,
         _orbit_id,
         _tile_id,
         _downloader_history_id
    from l1_tile_history
    inner join downloader_history on downloader_history.id = l1_tile_history.downloader_history_id
    inner join site on site.id = downloader_history.site_id
    cross join lateral (
        select
            (
                select value :: int as max_retries
                from site_config
                where site_config.site_id = downloader_history.site_id
                  and key = 'processor.l2a.optical.max-retries'
            ),
            (
                select value :: interval as retry_interval
                from site_config
                where site_config.site_id = downloader_history.site_id
                  and key = 'processor.l2a.optical.retry-interval'
            ),
            (
                select value :: boolean as s2_enabled
                from site_config
                where site_config.site_id = downloader_history.site_id
                  and key = 's2.enabled'
            ),
            (
                select value :: boolean as l8_enabled
                from site_config
                where site_config.site_id = downloader_history.site_id
                  and key = 'l8.enabled'
            )
    ) config
    where l1_tile_history.status_id = 2 -- failed
      and l1_tile_history.retry_count < config.max_retries
      and l1_tile_history.status_timestamp < now() - config.retry_interval
      and case downloader_history.satellite_id
              when 1 then config.s2_enabled
              when 2 then config.l8_enabled
              else false
      end
      and (
          site.enabled
          or exists (
              select *
              from downloader_history
              where downloader_history.status_id = 2 -- downloaded
                and l1_tile_history.tile_id = any(downloader_history.tiles)
                and l1_tile_history.orbit_id = downloader_history.orbit_id
                and exists (
                    select *
                    from site
                    where site.id = downloader_history.site_id
                      and site.enabled
                )
          )
      )
    order by l1_tile_history.status_timestamp
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
        inner join site on site.id = downloader_history.site_id
        cross join lateral (
                select unnest(tiles) as tile_id
            ) tile_ids
        cross join lateral (
            select
                (
                    select value as l2a_implementation
                    from site_config
                    where site_config.site_id = downloader_history.site_id
                    and key = 'processor.l2a.s2.implementation'
                ),
                (
                    select value :: int as max_retries
                    from site_config
                    where site_config.site_id = downloader_history.site_id
                    and key = 'processor.l2a.optical.max-retries'
                ),
                (
                    select value :: boolean as s2_enabled
                    from site_config
                    where site_config.site_id = downloader_history.site_id
                    and key = 's2.enabled'
                ),
                (
                    select value :: boolean as l8_enabled
                    from site_config
                    where site_config.site_id = downloader_history.site_id
                    and key = 'l8.enabled'
                )
        ) config
        where (
            config.l2a_implementation = 'sen2cor'
            and downloader_history.satellite_id = 1
            or not exists (
                select *
                from l1_tile_history
                where (l1_tile_history.satellite_id,
                    l1_tile_history.orbit_id,
                    l1_tile_history.tile_id) =
                    (downloader_history.satellite_id,
                    downloader_history.orbit_id,
                    tile_ids.tile_id)
                and (status_id = 1 or -- processing
                    retry_count < config.max_retries and status_id = 2 -- failed
                )
            )
        )
        and not exists (
            select *
            from l1_tile_history
            where (l1_tile_history.downloader_history_id, l1_tile_history.tile_id) = (downloader_history.id, tile_ids.tile_id)
        )
        and downloader_history.status_id in (2, 7) -- downloaded, processing
        and site.enabled
        and downloader_history.satellite_id in (1, 2) -- sentinel2, landsat8
        and case downloader_history.satellite_id
                when 1 then config.s2_enabled
                when 2 then config.l8_enabled
                else false
        end
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
