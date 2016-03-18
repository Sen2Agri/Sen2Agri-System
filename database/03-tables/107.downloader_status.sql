CREATE TABLE downloader_status
(
  id serial NOT NULL,
  status_description character varying NOT NULL,
  CONSTRAINT downloader_status_pkey PRIMARY KEY (id)
)
