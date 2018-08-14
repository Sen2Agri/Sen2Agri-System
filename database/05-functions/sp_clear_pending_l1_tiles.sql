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
