CREATE TABLE job_start_type
(
  id smallserial,
  name character varying NOT NULL,
  description character varying NOT NULL,
  CONSTRAINT job_start_type_pkey PRIMARY KEY (id)
)