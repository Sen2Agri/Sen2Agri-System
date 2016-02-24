CREATE TABLE config_metadata
(
  key character varying NOT NULL,
  friendly_name character varying NOT NULL DEFAULT '',
  type t_data_type NOT NULL,
  is_advanced boolean NOT NULL default false,
  config_category_id smallint NOT NULL,
  CONSTRAINT config_metadata_pkey PRIMARY KEY (key)
)