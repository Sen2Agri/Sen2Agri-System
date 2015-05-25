CREATE TABLE config
(
  key character varying NOT NULL,
  friendly_name character varying NOT NULL DEFAULT '',
  value character varying,
  type t_data_type NOT NULL,
  is_advanced boolean NOT NULL default false,
  config_category_id smallint NOT NULL,
  site_id smallint DEFAULT NULL,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_pkey PRIMARY KEY (key)
)