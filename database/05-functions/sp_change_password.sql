CREATE OR REPLACE FUNCTION "sp_changePassword"("userId" smallint, "oldPassword" character varying, "newPassword" character varying)
  RETURNS void AS
$BODY$UPDATE public.user
	     SET password = crypt($3, gen_salt('md5'))
	     WHERE id = $1 AND password = crypt($2, password)$BODY$
  LANGUAGE sql VOLATILE;
