create or replace function sp_delete_season(
    _season_id season.id%type
)
returns void as
$$
begin
    delete from scheduled_task_status where task_id in 
			(select id from scheduled_task where season_id = _season_id);
            
    delete from scheduled_task
    where season_id = _season_id;

    delete from season
    where id = _season_id;

    if not found then
        raise exception 'Invalid season % for site %', _name, _site_id;
    end if;
end;
$$
    language plpgsql volatile;
