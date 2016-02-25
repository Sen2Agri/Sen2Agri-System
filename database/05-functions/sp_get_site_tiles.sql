CREATE OR REPLACE FUNCTION sp_get_site_tiles(_site_id site.id%TYPE, _satellite_id satellite.id%TYPE)
RETURNS TABLE (
    tile_id CHARACTER VARYING
)
AS $$
BEGIN
    CASE _satellite_id
        WHEN 1 THEN -- sentinel2
            RETURN QUERY
                SELECT shape_tiles_s2.tile_id :: CHARACTER VARYING
                FROM site
                INNER JOIN shape_tiles_s2 ON ST_Intersects(shape_tiles_s2.geog, site.geog)
                WHERE site.id = _site_id;
        WHEN 2 THEN -- landsat8
            RETURN QUERY
                SELECT lpad(shape_tiles_l8.pr :: CHARACTER VARYING, 6, '0') :: CHARACTER VARYING
                FROM shape_tiles_l8
                INNER JOIN site ON ST_Intersects(site.geog, shape_tiles_l8.geog)
                WHERE site.id = _site_id;
    END CASE;
END;
$$
LANGUAGE plpgsql
STABLE;
