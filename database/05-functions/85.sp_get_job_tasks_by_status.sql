CREATE OR REPLACE FUNCTION sp_get_job_tasks_by_status(
IN _job_id int,
IN _status_list json
) RETURNS TABLE (
task_id int
) AS $$
BEGIN

RETURN QUERY SELECT id FROM task
WHERE job_id = _job_id
AND status_id IN (SELECT value::smallint FROM json_array_elements_text(_status_list));

END;
$$ LANGUAGE plpgsql;





