CREATE TABLE product
(
  id serial,
  processor_id smallint NOT NULL,
  product_type_id smallint NOT NULL,
  site_id smallint NOT NULL,
  full_path varchar NOT NULL,
  created timestamp with time zone NOT NULL DEFAULT now(),
  is_archived boolean DEFAULT FALSE,
  archived timestamp with time zone NOT NULL DEFAULT now(),
  CONSTRAINT product_pkey PRIMARY KEY (id)
)