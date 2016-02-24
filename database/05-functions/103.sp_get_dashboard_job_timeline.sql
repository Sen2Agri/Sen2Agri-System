CREATE OR REPLACE FUNCTION sp_get_dashboard_job_timeline(
    _job_id INT
)
RETURNS json
AS $$
DECLARE res JSON;
BEGIN
    WITH tasks AS (
        SELECT task.id AS id
        FROM task
        WHERE task.job_id = _job_id
    ),
    groups AS (
        SELECT task.module_short_name AS "content",
               task.id AS id
        FROM tasks
        INNER JOIN task ON task.id = tasks.id
    ),
    items AS (
        SELECT step.task_id AS "group",
               "name" AS "title",
               step.start_timestamp AS "start",
               COALESCE(step.end_timestamp, NOW()) AS "end"
        FROM step
        INNER JOIN tasks ON tasks.id = step.task_id
        WHERE step.start_timestamp IS NOT NULL
    ),
    result AS (
        SELECT (SELECT array_agg(groups) FROM groups) AS groups,
               (SELECT array_agg(items) FROM items) AS items
    )
    SELECT row_to_json(result)
    INTO res
    FROM result;

    RETURN res;
END;
$$ LANGUAGE plpgsql
STABLE;
