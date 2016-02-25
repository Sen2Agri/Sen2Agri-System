CREATE TABLE config_job
(
  job_id int NOT NULL,
  "key" CHARACTER VARYING NOT NULL,
  "value" character varying,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_job_pkey PRIMARY KEY (job_id, "key")
);
