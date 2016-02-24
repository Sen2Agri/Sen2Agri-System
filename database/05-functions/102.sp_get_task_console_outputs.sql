CREATE OR REPLACE FUNCTION sp_get_task_console_outputs(_task_id task.id%TYPE)
RETURNS TABLE (
    task_id step.task_id%TYPE,
    step_name step.name%TYPE,
    stdout_text step_resource_log.stdout_text%TYPE,
    stderr_text step_resource_log.stderr_text%TYPE
)
AS $$
BEGIN
    RETURN QUERY
        SELECT step.task_id,
               step.name,
               step_resource_log.stdout_text,
               step_resource_log.stderr_text
         FROM step
         INNER JOIN step_resource_log ON step_resource_log.task_id = step.task_id
                                     AND step_resource_log.step_name = step.name
         WHERE step.task_id = _task_id;
END;
$$
LANGUAGE plpgsql
STABLE;
