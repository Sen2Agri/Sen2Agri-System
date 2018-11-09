CREATE TABLE product
(
  id serial NOT NULL,
  product_type_id smallint NOT NULL,
  processor_id smallint NOT NULL,
  site_id smallint NOT NULL,
  full_path character varying NOT NULL,
  created_timestamp timestamp with time zone NOT NULL DEFAULT now(),
  inserted_timestamp timestamp with time zone NOT NULL DEFAULT now(),
  is_archived boolean DEFAULT false,
  archived_timestamp timestamp with time zone,
  name character varying(512),
  quicklook_image character varying(512),
  footprint polygon,
  job_id integer,
  geog geography,
  satellite_id integer,
  orbit_id integer,
  orbit_type_id smallint,
  tiles character varying[] NOT NULL,
  downloader_history_id int,
  CONSTRAINT product_pkey PRIMARY KEY (id)
);
