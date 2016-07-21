CREATE OR REPLACE FUNCTION sp_get_scheduled_processor_task()
  RETURNS TABLE(processor character varying, site character varying, repeat_type smallint, repeat_on_month_day smallint, priority smallint, first_run_time character varying) AS
$BODY$

BEGIN 

	RETURN QUERY 

	SELECT processor.name, site.name,scheduled_task.repeat_type,scheduled_task.repeat_on_month_day,scheduled_task.priority,scheduled_task.first_run_time

	FROM site

	JOIN scheduled_task ON site.id = scheduled_task.site_id

	JOIN processor ON scheduled_task.processor_id=processor.id

	WHERE site.enabled

	ORDER BY site.name;

END

$BODY$
  LANGUAGE plpgsql STABLE
