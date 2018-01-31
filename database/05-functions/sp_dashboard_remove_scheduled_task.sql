
CREATE OR REPLACE FUNCTION sp_dashboard_remove_scheduled_task(_schedule_id smallint)
  RETURNS void AS
$BODY$
BEGIN 

DELETE FROM scheduled_task WHERE id = _schedule_id;

DELETE FROM scheduled_task_status WHERE task_id = _schedule_id;

END;
$BODY$
  LANGUAGE plpgsql
