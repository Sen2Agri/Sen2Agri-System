create or replace function sp_clear_pending_fmask_tiles()
returns void
as
$$
begin
    delete
    from fmask_history
    where status_id = 1; -- processing
end;
$$ language plpgsql volatile;
