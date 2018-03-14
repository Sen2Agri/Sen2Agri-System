CREATE INDEX fki_fk_downloader_count_site
    ON public.downloader_count USING btree
    (site_id)
    TABLESPACE pg_default;