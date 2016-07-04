CREATE TABLE config_category
(
  id smallint not null,
  name character varying NOT NULL,
  display_order int NOT NULL default 0,
  allow_per_site_customization boolean default true,
  CONSTRAINT config_category_pkey PRIMARY KEY (id)
)
