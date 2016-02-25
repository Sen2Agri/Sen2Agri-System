CREATE TABLE site
(
  id smallserial,
  name character varying NOT NULL,
  short_name character varying,
  geog geography(polygon) NOT NULL,
  CONSTRAINT site_pkey PRIMARY KEY (id)
);
