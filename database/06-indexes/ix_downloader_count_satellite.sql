CREATE INDEX fki_fk_downloader_count_satellite
    ON public.downloader_count USING btree
    (satellite_id)
    TABLESPACE pg_default;
