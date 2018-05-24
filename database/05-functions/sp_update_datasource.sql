
CREATE OR REPLACE FUNCTION sp_update_datasource(
	_id smallint,
	_scope smallint,
	_enabled boolean,
	_fetch_mode smallint,
	_max_retries integer,
	_retry integer,
	_max_connections integer,
	_download_path character varying,
	_local_root character varying,
	_username character varying,
	_password character varying
	)
  RETURNS void AS
$BODY$
	BEGIN
	IF _id IS NOT NULL THEN
		UPDATE datasource
		SET enabled = _enabled,
			scope = _scope,
			fetch_mode = _fetch_mode,
			max_retries = _max_retries,
			retry_interval_minutes = _retry,
			max_connections = _max_connections,
			download_path = _download_path,
			local_root = _local_root,
			username = _username,
			passwrd = _password
		WHERE id = _id;
	END IF;

	END;
$BODY$
LANGUAGE plpgsql VOLATILE
