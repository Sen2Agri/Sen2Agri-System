CREATE OR REPLACE FUNCTION sp_authenticate(IN usr character varying, IN pwd text)
  RETURNS TABLE(user_id smallint, site_id integer[], role_id smallint, role_name character varying) AS
$BODY$SELECT u.id, u.site_id, r.id, r.name
  FROM "user" u
  INNER JOIN role r ON u.role_id=r.id
  WHERE login = $1 AND crypt($2, password) = password
$BODY$
  LANGUAGE sql VOLATILE;