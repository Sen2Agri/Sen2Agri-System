CREATE TABLE module
(
  short_name character varying,
  name character varying NOT NULL DEFAULT '',
  description character varying,
  CONSTRAINT module_pkey PRIMARY KEY (short_name)
)