create or replace function sp_get_season_scheduled_processors(
    _season_id season.id%type
)
returns table (
    processor_id processor.id%type,
    processor_name processor.name%type,
    processor_short_name processor.short_name%type
) as
$$
begin
    return query
        select
            processor.id,
            processor.name,
            processor.short_name
        from processor
        where exists(select *
                     from scheduled_task
                     where scheduled_task.season_id = _season_id
                       and scheduled_task.processor_id = processor.id)
        order by processor.short_name;
end;
$$
    language plpgsql stable;
