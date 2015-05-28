CREATE TABLE resource_log_entry_type
(
  id smallserial,
  name character varying NOT NULL,
  description character varying NOT NULL,
  CONSTRAINT resource_log_entry_type_pkey PRIMARY KEY (id)
)