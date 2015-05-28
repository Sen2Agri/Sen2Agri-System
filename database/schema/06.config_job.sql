CREATE TABLE config_job
(
  config_id int NOT NULL,
  job_id int NOT NULL,
  value character varying,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_job_pkey PRIMARY KEY (config_id, job_id)
)