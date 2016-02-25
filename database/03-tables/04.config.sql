CREATE TABLE config
(
  id serial,
  key character varying NOT NULL,
  site_id smallint DEFAULT NULL,
  value character varying NOT NULL,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_pkey PRIMARY KEY (id)
)