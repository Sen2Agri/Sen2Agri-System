-- Function: sp_get_dashboard_current_job_data(integer)

-- DROP FUNCTION sp_get_dashboard_current_job_data(integer);

CREATE OR REPLACE FUNCTION sp_get_dashboard_current_job_data(page integer DEFAULT 1)
  RETURNS json AS
$BODY$DECLARE current_job RECORD;
DECLARE temp_json json;
DECLARE temp_json2 json;
DECLARE oset int;
DECLARE lmt int;
BEGIN
	CREATE TEMP TABLE current_jobs (
		id int,
		processor character varying,
		site  character varying,
		triggered_by character varying,
		triggered_on character varying,
		status_id smallint,
		status character varying,
		tasks_completed int,
		tasks_remaining int,
		current_tasks json,
		actions json
	) ON COMMIT DROP;

	oset := ($1 - 1) * 5;
	lmt := 5;
	-- Get the current jobs
	INSERT INTO current_jobs (
		id, 
		processor,
		site,
		triggered_by,
		triggered_on,
		status_id,
		status,
		tasks_completed,
		tasks_remaining)
	SELECT 
		job.id,
		processor.name,
		site.name,
		CASE job.start_type_id WHEN 1 THEN 'Available Product'  WHEN 2 THEN 'User Request' WHEN 3 THEN 'Scheduler Request' END,
		to_char(job.submit_timestamp, 'YYYY-MM-DD HH:MI:SS'),
		job.status_id,
		activity_status.name,
		(SELECT count(*) FROM task WHERE task.job_id = job.id AND task.status_id IN (6,7,8)),	-- Finished, Cancelled, Error
		(SELECT count(*) FROM task WHERE task.job_id = job.id AND task.status_id NOT IN (6,7,8)) 
	FROM job 
		INNER JOIN processor ON job.processor_id = processor.id 
		INNER JOIN site ON job.site_id = site.id
		INNER JOIN activity_status ON job.status_id = activity_status.id
	WHERE job.status_id NOT IN (6,7,8) -- Finished, Cancelled, Error
	  AND EXISTS(SELECT * FROM task WHERE task.job_id = job.id) -- this is to prevent an empty dashboard display when there are inconsistencies in the job state
		ORDER BY job.id DESC
		OFFSET oset LIMIT lmt;

	-- For each of the current jobs, get the data for their ongoing tasks
	CREATE TEMP TABLE current_tasks (
		current_task_module character varying,
		current_task_steps_completed int,
		current_task_steps_remaining int
	) ON COMMIT DROP;

	FOR current_job IN SELECT * FROM current_jobs ORDER BY id LOOP
		-- Clear the temporary table
		DELETE FROM current_tasks;

		-- Get the current tasks for each job
		INSERT INTO current_tasks (
			current_task_module,
			current_task_steps_completed,
			current_task_steps_remaining
		)
		WITH modules as (SELECT DISTINCT task.module_short_name FROM task WHERE task.job_id = current_job.id)
			SELECT  modules.module_short_name,
				(select count(*) from task where task.job_id = current_job.id and task.module_short_name = modules.module_short_name AND task.status_id = 4),
				(select count(*) from task where task.job_id = current_job.id and task.module_short_name = modules.module_short_name)
			FROM modules
			ORDER BY modules.module_short_name;

			IF current_job.status_id != 5 /*Paused*/ THEN
				temp_json := json_build_array(1,3,4);
			ELSE
				temp_json := json_build_array(2,3,4);
			END IF;

			SELECT array_to_json(array_agg(row_to_json(current_task_details, true))) INTO temp_json2
			FROM (SELECT * FROM current_tasks) current_task_details;

			UPDATE current_jobs 
				SET 	current_tasks = temp_json2,
					actions = temp_json
				WHERE 	current_jobs.id = current_job.id;
		
	END LOOP;

	SELECT array_to_json(array_agg(row_to_json(current_job_details, true))) INTO temp_json
	FROM (SELECT * FROM current_jobs) AS current_job_details;

	temp_json2 := json_build_object('current_jobs', temp_json);

	RETURN temp_json2;

END;$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION sp_get_dashboard_current_job_data(integer)
  OWNER TO admin;
