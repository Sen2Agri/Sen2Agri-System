CREATE TABLE step_resource_log
(
  step_name character varying NOT NULL,
  task_id int NOT NULL,
  node_name character varying NOT NULL,
  entry_timestamp timestamp with time zone NOT NULL default now(),
  duration_ms bigint,
  user_cpu_ms bigint,
  system_cpu_ms bigint,
  max_rss_kb int,
  max_vm_size_kb int,
  disk_read_b bigint,
  disk_write_b bigint,
  stdout_text CHARACTER VARYING NOT NULL,
  stderr_text CHARACTER VARYING NOT NULL,
  CONSTRAINT step_resource_log_pkey PRIMARY KEY (step_name, task_id)
);
