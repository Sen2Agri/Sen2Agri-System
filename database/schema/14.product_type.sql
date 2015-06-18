CREATE TABLE product_type
(
  id smallserial,
  name character varying NOT NULL DEFAULT '',
  description character varying,
  CONSTRAINT product_type_pkey PRIMARY KEY (id)
)