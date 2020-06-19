CREATE OR REPLACE FUNCTION sp_get_job_steps(
IN _job_id int
) RETURNS TABLE (
task_id int,
module_short_name character varying,
step_name character varying,
preceding_task_ids integer[],
parameters json
) AS $$
BEGIN

RETURN QUERY
WITH job_tasks AS (
        SELECT task.id AS task_id, task.module_short_name as module_short_name, task.preceding_task_ids as preceding_task_ids
        FROM task
        WHERE task.job_id = _job_id
    )
SELECT job_tasks.task_id, job_tasks.module_short_name, step.name AS step_name, job_tasks.preceding_task_ids , step.parameters
FROM job_tasks INNER JOIN step ON job_tasks.task_id = step.task_id;

END;
$$ LANGUAGE plpgsql;





