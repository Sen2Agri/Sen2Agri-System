CREATE UNIQUE INDEX ix_config_key_site_id ON config("key", COALESCE(site_id, -1));
