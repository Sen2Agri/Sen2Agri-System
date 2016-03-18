CREATE OR REPLACE FUNCTION sp_dashboard_update_scheduled_task(
    _schedule_id smallint,
    _repeat_type smallint,
    _repeat_after_days smallint,
    _repeat_on_month_day smallint,
    _first_run_time character varying)
  RETURNS void AS
$BODY$
BEGIN 

UPDATE scheduled_task
SET	repeat_type=_repeat_type,
	repeat_after_days=_repeat_after_days,
	repeat_on_month_day=_repeat_on_month_day,
	first_run_time = _first_run_time
WHERE id = _schedule_id;

END;
$BODY$
  LANGUAGE plpgsql