ALTER TABLE downloader_history ADD CONSTRAINT downloader_history_status_id_fkey FOREIGN KEY (status_id) REFERENCES downloader_status (id) MATCH SIMPLE
