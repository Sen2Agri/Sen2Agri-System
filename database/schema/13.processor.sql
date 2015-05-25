CREATE TABLE processor
(
  id smallserial,
  name character varying NOT NULL DEFAULT '',
  description character varying,
  CONSTRAINT processor_pkey PRIMARY KEY (id)
)