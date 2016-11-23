CREATE OR REPLACE FUNCTION sp_get_intersecting_tiles(
    _satellite_id satellite.id%TYPE,
    _tile_id TEXT
)
RETURNS TABLE (
    satellite_id satellite.id%TYPE,
    tile_id TEXT
)
AS $$
DECLARE _geog GEOGRAPHY;
BEGIN
    CASE _satellite_id
        WHEN 1 THEN
        BEGIN
            _geog := (SELECT geog
                      FROM shape_tiles_s2
                      WHERE shape_tiles_s2.tile_id = _tile_id);
        END;
        WHEN 2 THEN
        BEGIN
            _geog := (SELECT geog
                      FROM shape_tiles_l8
                      WHERE pr = _tile_id :: INT);
        END;
    END CASE;

    RETURN QUERY
        SELECT 1 AS satellite_id,
               shape_tiles_s2.tile_id :: TEXT
        FROM shape_tiles_s2
        WHERE ST_Intersects(shape_tiles_s2.geog, _geog)
        UNION
        SELECT 2 AS satellite_id,
               lpad(shape_tiles_l8.pr :: TEXT, 6, '0')
        FROM shape_tiles_l8
        WHERE ST_Intersects(shape_tiles_l8.geog, _geog)
        ORDER BY satellite_id, tile_id;
END;
$$
LANGUAGE plpgsql
STABLE;