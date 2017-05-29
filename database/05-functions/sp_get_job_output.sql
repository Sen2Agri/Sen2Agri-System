create or replace function sp_get_job_output(
    _job_id job.id%type
) returns table (
    step_name step.name%type,
    command text,
    stdout_text step_resource_log.stdout_text%type,
    stderr_text step_resource_log.stderr_text%type,
    exit_code step.exit_code%type
) as
$$
begin
    return query
        select step.name,
               array_to_string(array_prepend(config.value :: text, array(select json_array_elements_text(json_extract_path(step.parameters, 'arguments')))), ' ') as command,
               step_resource_log.stdout_text,
               step_resource_log.stderr_text,
               step.exit_code
        from task
        inner join step on step.task_id = task.id
        left outer join step_resource_log on step_resource_log.step_name = step.name and step_resource_log.task_id = task.id
        left outer join config on config.site_id is null and config.key = 'executor.module.path.' || task.module_short_name
        where task.job_id = _job_id
        order by step.submit_timestamp;
end;
$$
    language plpgsql stable;
