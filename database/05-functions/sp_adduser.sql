CREATE OR REPLACE FUNCTION sp_adduser(
    "userName" character varying, 
    email character varying, 
    pwd text, 
    "roleId" smallint DEFAULT 2, 
    "siteId" integer[] DEFAULT NULL::integer[])
  RETURNS smallint AS
$BODY$INSERT INTO "user"(
            id, login, email, role_id, site_id, password)
    VALUES (
    COALESCE((SELECT MAX(id) FROM public.user) :: integer, 0) + 1, 
    $1, $2, $4, $5,
    crypt($3, gen_salt('md5')));
SELECT id FROM public.user WHERE login = $1;$BODY$
  LANGUAGE sql VOLATILE;
