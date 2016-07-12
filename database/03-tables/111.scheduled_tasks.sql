CREATE TABLE scheduled_task
(
  id smallserial NOT NULL,
  name character varying NOT NULL,
  processor_id smallint NOT NULL,
  site_id smallint NOT NULL,
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
  id smallserial NOT NULL,
  task_id smallint NOT NULL,

  next_schedule character varying,

  last_scheduled_run character varying,
  last_run_timestamp character varying,
  last_retry_timestamp character varying,

  estimated_next_run_time character varying,

  CONSTRAINT scheduled_task_status_pkey PRIMARY KEY (id)
);
