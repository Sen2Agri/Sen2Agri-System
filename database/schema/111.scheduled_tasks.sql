CREATE TABLE scheduled_task
(
  id smallserial,
  name character varying NOT NULL,
  processor_id smallserial,
  site_id smallserial,
  processor_params character varying,

  repeat_type smallint,
  repeat_after_days smallint,
  repeat_on_month_day smallint,
  
  retry_seconds integer,

  priority smallint,
  
  first_run_time character varying,
    
  CONSTRAINT scheduled_task_pkey PRIMARY KEY (id)
);

CREATE TABLE scheduled_task_status
(
  id smallserial,
  task_id smallserial,

  next_schedule character varying,

  last_scheduled_run character varying,
  last_run_timestamp character varying,
  last_retry_timestamp character varying,

  estimated_next_run_time character varying,

  CONSTRAINT scheduled_task_status_pkey PRIMARY KEY (id)
);

