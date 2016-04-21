CREATE OR REPLACE FUNCTION sp_downloader_get_sites(
)
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
               (SELECT array_agg(polys.geog)
                FROM (
                    SELECT ST_AsText((ST_Dump(site.geog :: geometry)).geom :: geography) AS geog
                ) polys) AS geog
        FROM site;
END
$$
LANGUAGE plpgsql STABLE;
