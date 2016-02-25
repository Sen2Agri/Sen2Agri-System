CREATE OR REPLACE FUNCTION sp_get_sites()
RETURNS TABLE (
    id site.id%TYPE,
    "name" site."name"%TYPE,
    "short_name" site."short_name"%TYPE
)
AS $$
BEGIN
    RETURN QUERY
        SELECT site.id,
               site.name,
               site.short_name
        FROM site
        ORDER BY site.name;
END
$$
LANGUAGE plpgsql
STABLE;
