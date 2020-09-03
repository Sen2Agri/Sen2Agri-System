create or replace function sp_mark_fmask_l1_tile_failed(
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
