CREATE TABLE activity_status
(
  id smallint NOT NULL,
  name character varying NOT NULL DEFAULT ''::character varying,
  description character varying,
  CONSTRAINT activity_status_pkey PRIMARY KEY (id)
)