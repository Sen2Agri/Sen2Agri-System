CREATE TABLE product
(
  id smallserial,
  name character varying NOT NULL DEFAULT '',
  description character varying,
  CONSTRAINT product_pkey PRIMARY KEY (id)
)