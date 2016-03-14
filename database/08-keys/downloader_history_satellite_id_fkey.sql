ALTER TABLE downloader_history ADD CONSTRAINT downloader_history_satellite_id_fkey FOREIGN KEY (satellite_id) REFERENCES satellite (id) MATCH SIMPLE
