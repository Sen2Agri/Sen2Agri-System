create or replace function sp_get_season_scheduled_processors(
    _season_id season.id%type
)
returns table (
    processor_id processor.id%type
) as
$$
begin
    return query
        select distinct
            scheduled_task.processor_id
        from scheduled_task
        where scheduled_task.season_id = _season_id;
end;
$$
    language plpgsql stable;
