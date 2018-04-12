CREATE OR REPLACE FUNCTION sp_get_all_users()
RETURNS TABLE(user_id smallint, user_name character varying, email character varying, role_id smallint, role_name character varying, site_id integer[], site_name text ) AS
$BODY$
BEGIN
    RETURN QUERY 
    select u.id as user_id, 
	u.login as user_name,
	u. email,
	u.role_id,
	r.name as role_name, 
	u.site_id,
	(SELECT array_to_string(array_agg(name),', ') from site where id = ANY(u.site_id::int[]) ) as site_name
	--(select array(select name from site where array[id]::integer[] @> u.site_id )) as site_name
	/*CASE WHEN (select array(select name from site where id = ANY(u.site_id::int[])) ) = '{}'
		THEN NULL
	ELSE (select array(select name from site where id = ANY(u.site_id::int[]))) 
	END as site_name	*/
	/*(SELECT json_agg(siteName)
	FROM (SELECT name as siteName from site where id = ANY(u.site_id::int[]) ) as sites) as site_name*/
    FROM "user" u 
    INNER JOIN role r ON u.role_id = r.id;

END;
$BODY$
LANGUAGE plpgsql VOLATILE