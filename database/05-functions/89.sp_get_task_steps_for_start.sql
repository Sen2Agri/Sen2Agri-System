CREATE OR REPLACE FUNCTION sp_get_task_steps_for_start(
IN _task_id int
) RETURNS TABLE (
task_id int,
module_short_name character varying,
step_name character varying,
parameters json
) AS $$
BEGIN

RETURN QUERY SELECT task.id AS task_id, task.module_short_name, step.name AS step_name, step.parameters
FROM task INNER JOIN step ON task.id = step.task_id
WHERE task.id = _task_id
AND step.status_id = 1;

END;
$$ LANGUAGE plpgsql;
