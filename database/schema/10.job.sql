CREATE TABLE job
(
  id serial,
  processor_id smallint NOT NULL,
  product_id smallint NOT NULL,
  site_id smallint NOT NULL,
  start_type_id smallint NOT NULL,
  input_path character varying,
  output_path character varying,
  submit_timestamp timestamp with time zone NOT NULL default now(),
  start_timestamp timestamp with time zone,
  end_timestamp timestamp with time zone,
  tiles_total smallint NOT NULL,
  tiles_processed smallint NOT NULL,
  CONSTRAINT job_pkey PRIMARY KEY (id)
)