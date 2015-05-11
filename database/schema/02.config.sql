CREATE TABLE config
(
  key character varying NOT NULL,
  value character varying,
  type t_data_types,
  last_updated timestamp with time zone,
  CONSTRAINT config_pkey PRIMARY KEY (key)
)