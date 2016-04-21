CREATE OR REPLACE FUNCTION sp_get_site_tiles(_site_id site.id%TYPE, _satellite_id satellite.id%TYPE)
RETURNS TABLE (
    tile_id text
)
AS $$
BEGIN
    CASE _satellite_id
        WHEN 1 THEN -- sentinel2
            RETURN QUERY
                WITH geog_tiles AS (
                    SELECT array_agg(shape_tiles_s2.tile_id :: text) as tiles
                    FROM shape_tiles_s2
                    INNER JOIN site ON ST_Intersects(site.geog, shape_tiles_s2.geog)
                    WHERE site.id = _site_id
                )
                SELECT unnest(geog_tiles.tiles)
                FROM geog_tiles
                INTERSECT
                SELECT unnest(COALESCE(site_tiles.tiles, geog_tiles.tiles))
                from geog_tiles
                LEFT OUTER JOIN site_tiles ON site_tiles.site_id = _site_id
                                          AND site_tiles.satellite_id = 1; -- landsat8
        WHEN 2 THEN -- landsat8
            RETURN QUERY
                WITH geog_tiles AS (
                    SELECT array_agg(lpad(shape_tiles_l8.pr :: text, 6, '0')) as tiles
                    FROM shape_tiles_l8
                    INNER JOIN site ON ST_Intersects(site.geog, shape_tiles_l8.geog)
                    WHERE site.id = _site_id
                )
                SELECT unnest(geog_tiles.tiles)
                FROM geog_tiles
                INTERSECT
                SELECT unnest(COALESCE(site_tiles.tiles, geog_tiles.tiles))
                from geog_tiles
                LEFT OUTER JOIN site_tiles ON site_tiles.site_id = _site_id
                                          AND site_tiles.satellite_id = 2; -- landsat8
    END CASE;
END;
$$
LANGUAGE plpgsql
STABLE;
