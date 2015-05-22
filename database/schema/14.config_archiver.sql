CREATE TABLE config_archiver
(
  id serial,
  processor_id smallint NOT NULL,
  product_id smallint NOT NULL,
  min_age smallint NOT NULL,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_archiver_pkey PRIMARY KEY (id)
)