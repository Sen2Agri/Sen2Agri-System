CREATE TABLE config_subscription
(
  key character varying NOT NULL,
  id_job int NOT NULL,
  value character varying,
  last_updated timestamp with time zone NOT NULL default now(),
  CONSTRAINT config_subscription_pkey PRIMARY KEY (key)
)