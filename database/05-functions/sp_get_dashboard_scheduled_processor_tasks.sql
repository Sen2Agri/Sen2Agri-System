CREATE OR REPLACE FUNCTION sp_get_dashboard_scheduled_processor_tasks(processor_id smallint DEFAULT NULL::smallint)
  RETURNS json AS
$BODY$

DECLARE return_string text;

BEGIN

	WITH data(proc_name,site_name,repeat_type,repeat_on_month_day,priority,first_run_time) AS (



		SELECT  processor.name, 

			site.name,

			scheduled_task.repeat_type,

			scheduled_task.repeat_on_month_day,

			scheduled_task.priority,

			scheduled_task.first_run_time

		FROM site

			JOIN scheduled_task ON site.id = scheduled_task.site_id

			JOIN processor ON scheduled_task.processor_id=processor.id

		WHERE ($1 IS NULL OR processor.ID = $1)

	)

	SELECT array_to_json(array_agg(row_to_json(data)),true) INTO return_string FROM data;

	RETURN return_string::json;

END

$BODY$
  LANGUAGE plpgsql VOLATILE
