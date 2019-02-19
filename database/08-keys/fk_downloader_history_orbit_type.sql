ALTER TABLE downloader_history ADD CONSTRAINT fk_product_orbit_type FOREIGN KEY (orbit_type_id) REFERENCES public.orbit_type (id);
