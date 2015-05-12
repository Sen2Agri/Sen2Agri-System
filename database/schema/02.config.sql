CREATE TABLE config
(
  key character varying NOT NULL,
  value character varying,
  type t_data_types NOT NULL,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_pkey PRIMARY KEY (key)
)