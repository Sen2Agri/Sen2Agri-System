CREATE OR REPLACE FUNCTION sp_get_scheduled_tasks()
RETURNS TABLE (
    id scheduled_task.id%TYPE,
    name scheduled_task.name%TYPE,
    
    processor_id scheduled_task.processor_id%TYPE,
    site_id scheduled_task.site_id%TYPE,    
    processor_params scheduled_task.processor_params%TYPE,

    repeat_type scheduled_task.repeat_type%TYPE,
    repeat_after_days scheduled_task.repeat_after_days%TYPE,
    repeat_on_month_day scheduled_task.repeat_on_month_day%TYPE,
    retry_seconds scheduled_task.retry_seconds%TYPE,
    
    priority scheduled_task.priority%TYPE,
  
    first_run_time scheduled_task.first_run_time%TYPE,

    status_id scheduled_task_status.id%TYPE,
    next_schedule scheduled_task_status.next_schedule%TYPE,
    last_scheduled_run scheduled_task_status.last_scheduled_run%TYPE,
    last_run_timestamp scheduled_task_status.last_run_timestamp%TYPE,
    last_retry_timestamp scheduled_task_status.last_retry_timestamp%TYPE,
    estimated_next_run_time scheduled_task_status.estimated_next_run_time%TYPE,
    
)
AS $$
BEGIN
    RETURN QUERY
        SELECT *
        FROM scheduled_task JOIN scheduled_task_status ON (scheduled_task.id == scheduled_task_status.task_id)
        ;
END
$$
LANGUAGE plpgsql
STABLE;
