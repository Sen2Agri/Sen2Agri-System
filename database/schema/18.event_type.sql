CREATE TABLE event_type
(
  id smallint NOT NULL,
  name character varying NOT NULL DEFAULT ''::character varying,
  description character varying,
  CONSTRAINT event_type_pkey PRIMARY KEY (id)
)