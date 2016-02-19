CREATE OR REPLACE FUNCTION sp_insert_scheduled_task(
    _name character varying,
    _processor_id integer,
    _site_id integer,
    _repeat_type smallint,
    _repeat_after_days smallint,
    _repeat_on_month_day smallint,
    _first_run_time character varying,
    _retry_seconds  integer,
    _priority smallint,
    _processor_params json)
  RETURNS integer AS
$BODY$
DECLARE _return_id int;
BEGIN

	INSERT INTO scheduled_task(
		name,
		processor_id,
		site_id,
		repeat_type,
		repeat_after_days,
		repeat_on_month_day,
		first_run_time,
		retry_seconds,
		priority,
		processor_params)
	VALUES (
		_name,
		_processor_id,
		_site_id,
		_repeat_type,
		_repeat_after_days,
		_repeat_on_month_day,
		_first_run_time,
		_retry_seconds,
		_priority,
		_processor_params
	) RETURNING id INTO _return_id;

	INSERT INTO scheduled_task_status(
		task_id,
		next_schedule,
		last_scheduled_run,
		last_run_timestamp,
		last_retry_timestamp,
		estimated_next_run_time)
	VALUES (
		_return_id,
		_first_run_time,
		'0',
		'0',
		'0',
		'0'
	);
    
	RETURN _return_id;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION sp_insert_scheduled_task(character varying,integer,integer,smallint,smallint,smallint,character varying,integer,smallint,json)
  OWNER TO admin;
