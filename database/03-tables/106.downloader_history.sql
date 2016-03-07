CREATE TABLE downloader_history
(
  id serial,  
  site_id smallint NOT NULL,
  satellite_id smallint NOT NULL references satellite(id),
  product_name varchar NOT NULL,
  product_date timestamp with time zone NOT NULL DEFAULT now(),	
  full_path varchar NOT NULL,
  created_timestamp timestamp with time zone NOT NULL DEFAULT now(), 
  
  CONSTRAINT downloader_history_pkey PRIMARY KEY (id)
)
