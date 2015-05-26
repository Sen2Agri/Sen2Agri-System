CREATE TABLE config_job
(
  key character varying NOT NULL,
  id_job int NOT NULL,
  value character varying,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_job_pkey PRIMARY KEY (key)
)