  CREATE OR REPLACE FUNCTION sp_set_user_password(
	IN user_name character varying,
	IN email character varying,
	IN pwd text	
  )RETURNS character varying AS
  $BODY$
  DECLARE user_id smallint;

  BEGIN 
	SELECT id into user_id FROM "user" WHERE "user".login = $1 AND "user".email = $2;

	IF user_id IS NOT NULL THEN
		IF char_length(trim(pwd))>0 THEN

			UPDATE "user"
			     SET password = crypt($3, gen_salt('md5'))
			     WHERE id = user_id ;--AND password = crypt(user_pwd, password);
			RETURN 1;
		ELSE 
			RETURN 0;
		END IF;
	ELSE RETURN 2;
	END IF;
	
  END;
  $BODY$
  LANGUAGE plpgsql VOLATILE