create or replace function sp_mark_fmask_l1_tile_done(
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
