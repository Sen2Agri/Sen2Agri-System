CREATE OR REPLACE FUNCTION sp_authenticate(IN usr character varying, IN pwd text)
  RETURNS TABLE(user_id smallint, site_id smallint) AS
$BODY$SELECT id, site_id
  FROM "user"
  WHERE login = $1 AND crypt($2, password) = password
$BODY$
  LANGUAGE sql VOLATILE;
