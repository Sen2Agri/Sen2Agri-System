CREATE TABLE role
(
  id smallint NOT NULL,
  name character varying(10) NOT NULL,
  description character varying(50) NOT NULL,
  CONSTRAINT role_pkey PRIMARY KEY (id),
  CONSTRAINT role_name_key UNIQUE (name)
);
