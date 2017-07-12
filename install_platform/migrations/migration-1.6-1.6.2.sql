begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version = '1.6') then
            raise notice 'upgrading from 1.6 to 1.6.2';

            raise notice 'applying 621e1c15b38b1bb2bd6b03661eced1d3a5ab204b';
            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_last_l2a_product(
                    _site_id site.id%TYPE,
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
                        WHERE product.site_id = _site_id
                        AND product.product_type_id = 1   -- only L2A
                        AND product.satellite_id = _satellite_id
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
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := 'update meta set version = ''1.6.2'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;
