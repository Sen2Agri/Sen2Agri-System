CREATE TABLE processor
(
  id smallint not null,
  name character varying NOT NULL DEFAULT '',
  description character varying,
  short_name character varying,
  label character varying,
  CONSTRAINT processor_pkey PRIMARY KEY (id)
)
