CREATE OR REPLACE FUNCTION sp_get_products_for_tile(_tile_id CHARACTER VARYING, _satellite_id satellite.id%TYPE)
RETURNS TABLE (
    full_path product.full_path%TYPE,
    product_date product.created_timestamp%TYPE
)
AS $$
DECLARE _geog GEOGRAPHY;
BEGIN
    CASE _satellite_id
        WHEN 1 THEN -- sentinel2
            _geog := (SELECT shape_tiles_s2.geog FROM shape_tiles_s2 WHERE tile_id = _tile_id);
        WHEN 2 THEN -- landsat8
            _geog := (SELECT shape_tiles_l8 FROM shape_tiles_l8 WHERE shape_tiles_l8.pr = _tile_id :: INT);
    END CASE;

    RETURN QUERY
        SELECT product.full_path,
               product.created_timestamp
        FROM product
        WHERE product.satellite_id = _satellite_id AND
              ST_Intersects(product.geog, _geog);
END;
$$
LANGUAGE plpgsql
STABLE;
