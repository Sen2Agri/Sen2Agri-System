CREATE TABLE job
(
  id serial NOT NULL,
  processor_id smallint NOT NULL,
  site_id smallint NOT NULL,
  start_type_id smallint NOT NULL,
  parameters varchar,
  submit_timestamp timestamp with time zone NOT NULL DEFAULT now(),
  start_timestamp timestamp with time zone,
  end_timestamp timestamp with time zone,
  status_id smallint NOT NULL,
  status_timestamp timestamp with time zone NOT NULL DEFAULT now(),
  CONSTRAINT job_pkey PRIMARY KEY (id)
)