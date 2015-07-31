CREATE TABLE product
(
  id serial,
  product_type_id smallint NOT NULL,
  processor_id smallint NOT NULL,
  task_id int NOT NULL,
  site_id smallint NOT NULL,
  full_path varchar NOT NULL,
  created_timestamp timestamp with time zone NOT NULL DEFAULT now(),
  is_archived boolean DEFAULT FALSE,
  archived_timestamp timestamp with time zone,
  CONSTRAINT product_pkey PRIMARY KEY (id)
)