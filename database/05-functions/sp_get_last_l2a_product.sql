CREATE OR REPLACE FUNCTION sp_get_last_l2a_product(
    _tile_id CHARACTER VARYING,
    _satellite_id satellite.id%TYPE,
    _orbit_id product.orbit_id%TYPE,
    _l1c_date product.created_timestamp%TYPE
) RETURNS TABLE (
    path product.full_path%TYPE,
    "date" product.created_timestamp%TYPE
)
AS $$
BEGIN
    RETURN QUERY
        SELECT product.full_path path,
               product.created_timestamp "date"
        FROM product
        WHERE product.satellite_id = _satellite_id
          AND product.created_timestamp < _l1c_date
          AND product.tiles @> ARRAY[_tile_id]
          AND (_satellite_id <> 1 -- sentinel2
            OR product.orbit_id = _orbit_id)
        ORDER BY product.created_timestamp DESC
        LIMIT 1;
END;
$$
LANGUAGE plpgsql
STABLE;
