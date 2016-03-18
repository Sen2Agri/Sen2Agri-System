CREATE OR REPLACE FUNCTION sp_get_sites(IN _siteid smallint DEFAULT NULL::smallint)
  RETURNS TABLE(id smallint, name character varying, short_name character varying) AS
$BODY$
BEGIN
   RETURN QUERY
   
        SELECT site.id,
               site.name,
               site.short_name
        FROM site
        WHERE($1 IS NULL OR site.id = $1)
        ORDER BY site.name;

 
END
$BODY$
  LANGUAGE plpgsql