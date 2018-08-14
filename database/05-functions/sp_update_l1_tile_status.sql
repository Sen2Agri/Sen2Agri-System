create or replace function sp_update_l1_tile_status(
    _downloader_history_id int
)
returns int
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
        return _downloader_history_id;
    else
        return null;
    end if;
end;
$$ language plpgsql volatile;
