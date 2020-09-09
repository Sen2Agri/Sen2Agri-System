create or replace function sp_mark_l1_tile_failed(
    _downloader_history_id int,
    _tile_id text,
    _reason text,
    _should_retry boolean,
    _cloud_coverage int,
    _snow_coverage int
)
returns boolean
as
$$
begin
	if (select current_setting('transaction_isolation') not ilike 'serializable') then
		raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
	end if;

    update l1_tile_history
    set status_id = 2, -- failed
        status_timestamp = now(),
        retry_count = case _should_retry
            when true then retry_count + 1
            else 4
        end,
        failed_reason = _reason,
        cloud_coverage = _cloud_coverage,
        snow_coverage = _snow_coverage
    where (downloader_history_id, tile_id) = (_downloader_history_id, _tile_id);

    return sp_update_l1_tile_status(_downloader_history_id);
end;
$$ language plpgsql volatile;
