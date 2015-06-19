CREATE TABLE task
(
  id serial NOT NULL,
  job_id int NOT NULL,
  module_short_name character varying NOT NULL,
  parameters character varying,
  submit_timestamp timestamp with time zone NOT NULL DEFAULT now(),
  start_timestamp timestamp with time zone,
  end_timestamp timestamp with time zone,
  status_id smallint NOT NULL,
  status_timestamp timestamp with time zone NOT NULL DEFAULT now(),
  CONSTRAINT task_pkey PRIMARY KEY (id)
)