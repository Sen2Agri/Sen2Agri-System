CREATE TABLE config_metadata
(
  key character varying NOT NULL,
  friendly_name character varying NOT NULL DEFAULT '',
  type t_data_type NOT NULL,
  is_advanced boolean NOT NULL DEFAULT false,
  config_category_id smallint NOT NULL,
  is_site_visible boolean NOT NULL DEFAULT false,
  label character varying,
  values json,
  CONSTRAINT config_metadata_pkey PRIMARY KEY (key)
)