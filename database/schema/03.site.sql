CREATE TABLE site
(
  id smallserial,
  name character varying NOT NULL,
  short_name character varying,
  CONSTRAINT site_pkey PRIMARY KEY (id)
)