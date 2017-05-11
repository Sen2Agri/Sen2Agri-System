CREATE OR REPLACE FUNCTION sp_downloader_get_sites()
RETURNS TABLE (
    id site.id%TYPE,
    name site.name%TYPE,
    short_name site.short_name%TYPE,
    geog text[]
)
AS $$
BEGIN
    RETURN QUERY
        SELECT site.id,
               site.name,
               site.short_name,
               (SELECT array_agg(ST_AsText(ST_MakeValid(ST_SnapToGrid(polys.geog, 0.001))))
                FROM (
                    SELECT (ST_Dump(site.geog :: geometry)).geom AS geog
                ) polys)
        FROM site
	WHERE site.enabled;
END
$$
LANGUAGE plpgsql STABLE;
