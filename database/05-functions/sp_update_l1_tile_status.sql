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
          and (l1_tile_history.status_id = 3 -- done
            or l1_tile_history.status_id = 2 -- failed
                and l1_tile_history.retry_count >= (
                    select
                        coalesce(
                            (
                                select value
                                from config
                                where key = 'processor.l2a.optical.max-retries'
                                and site_id = (
                                    select site_id
                                    from downloader_history
                                    where id = _downloader_history_id)
                            ), (
                                select value
                                from config
                                where key = 'processor.l2a.optical.max-retries'
                                and site_id is null
                            )
                        ) :: int
                )
          )
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
