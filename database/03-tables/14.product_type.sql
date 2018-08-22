CREATE TABLE product_type
(
  id smallint NOT NULL,
  name character varying NOT NULL DEFAULT '',
  description character varying,
  is_raster boolean NOT NULL DEFAULT true,
  CONSTRAINT product_type_pkey PRIMARY KEY (id)
)
