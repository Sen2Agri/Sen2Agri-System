CREATE OR REPLACE FUNCTION sp_adduser("userName" character varying, email character varying, pwd text, "roleId" smallint DEFAULT 2, "siteId" smallint DEFAULT NULL::smallint)
  RETURNS smallint AS
$BODY$INSERT INTO "user"(
            id, login, email, role_id, site_id, password)
    VALUES (
    (SELECT MAX(id)+1 FROM public.user), 
    $1, $2, $4, $5,
    crypt($3, gen_salt('md5')));
SELECT id FROM public.user WHERE login = $1;$BODY$
  LANGUAGE sql VOLATILE
