CREATE TABLE event_type
(
  id smallserial NOT NULL,
  name character varying NOT NULL DEFAULT ''::character varying,
  description character varying,
  CONSTRAINT event_type_pkey PRIMARY KEY (id)
)