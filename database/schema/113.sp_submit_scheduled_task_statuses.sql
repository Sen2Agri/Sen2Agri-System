CREATE OR REPLACE FUNCTION sp_submit_scheduled_tasks_statuses(
IN _statuses json
) RETURNS void AS $$
BEGIN


        UPDATE scheduled_task_status SET
		next_schedule = params.next_schedule,
		last_scheduled_run = params.last_scheduled_run,
		last_run_timestamp = params.last_run_timestamp,
		last_retry_timestamp = params.last_retry_timestamp,
		estimated_next_run_time = params.estimated_next_run_time
		
	FROM  json_populate_recordset(null::params, _statuses) AS params
	
        WHERE id = params.id;

END;
$$ LANGUAGE plpgsql;