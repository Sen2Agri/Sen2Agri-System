-- Table: datasource

-- DROP TABLE datasource;

CREATE TABLE downloader_count
(
    site_id smallint NOT NULL,
    satellite_id smallint NOT NULL,
    product_count integer NOT NULL,
    start_date date NOT NULL,
    end_date date NOT NULL,
    last_updated timestamp with time zone DEFAULT now(),
    CONSTRAINT pk_donwloader_count PRIMARY KEY (site_id, satellite_id, start_date, end_date),
    CONSTRAINT fk_downloader_count_satellite FOREIGN KEY (satellite_id)
        REFERENCES satellite (id) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION,
    CONSTRAINT fk_downloader_count_site FOREIGN KEY (site_id)
        REFERENCES site (id) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE downloader_count
    OWNER to admin;

