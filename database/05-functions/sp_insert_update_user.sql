CREATE OR REPLACE FUNCTION sp_insert_update_user(
    _username character varying,
    _email character varying,
    _id smallint DEFAULT NULL::smallint,
    _roleid smallint DEFAULT 2,
    _siteid integer[] DEFAULT NULL::integer[])
  RETURNS void AS
$BODY$
BEGIN

    IF _id IS NOT NULL THEN
        UPDATE public.user
	    SET login = _username,
	    email = _email,
	    role_id = _roleid,
	    site_id = _siteid
	    WHERE id = _id ;
    ELSE 
	    INSERT INTO public.user (id, login, email, role_id, site_id)
	    VALUES (COALESCE((SELECT MAX(id) FROM public.user) :: integer, 0) + 1, 
	    _username, _email, _roleid, _siteid);
    END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE;
