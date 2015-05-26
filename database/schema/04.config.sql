CREATE TABLE config
(
  key character varying NOT NULL,
  site_id smallint DEFAULT NULL,
  value character varying,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_pkey PRIMARY KEY (key, site_id)
)