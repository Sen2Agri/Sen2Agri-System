CREATE TABLE event
(
  id serial,
  type_id smallint NOT NULL,
  data character varying,
  submitted_timestamp timestamp with time zone DEFAULT now(),
  processing_started_timestamp timestamp with time zone,
  processing_completed_timestamp timestamp with time zone,
  CONSTRAINT event_pkey PRIMARY KEY (id)
)
