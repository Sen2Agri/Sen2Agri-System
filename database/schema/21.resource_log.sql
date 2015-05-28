CREATE TABLE resource_log
(
  id serial,
  job_id int DEFAULT NULL,
  date_time timestamp with time zone NOT NULL default now(),
  entry_type_id smallint NOT NULL,
  cpu_time time,
  memory decimal,
  disk_free decimal,
  ram_free decimal,
  CONSTRAINT resource_log_pkey PRIMARY KEY (id)
)