-- Function: sp_get_dashboard_products(smallint, smallint)

-- DROP FUNCTION sp_get_dashboard_products(smallint, smallint);

CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
    _site_id smallint DEFAULT NULL::smallint,
    _processor_id smallint DEFAULT NULL::smallint)
  RETURNS TABLE (
--     id product.id%type,
--     satellite_id product.satellite_id%type,
--     product product.name%type,
--     product_type product_type.name%type,
--     product_type_description product_type.description%type,
--     processor processor.name%type,
--     site site.name%type,
--     full_path product.full_path%type,
--     quicklook_image product.quicklook_image%type,
--     footprint product.footprint%type,
--     created_timestamp product.created_timestamp%type,
--     site_coord text
    json json
) AS
$BODY$
DECLARE summer_season_start_str text;
DECLARE summer_season_end_str text;
DECLARE winter_season_start_str text;
DECLARE winter_season_end_str text;
DECLARE summer_season_start date;
DECLARE summer_season_end date;
DECLARE winter_season_start date;
DECLARE winter_season_end date;
DECLARE q text;
BEGIN
    q := $sql$
        WITH site_names(id, name, geog, row) AS (
                select id, name, st_astext(geog), row_number() over (order by name)
                from site
            ),
            product_type_names(id, name, description, row) AS (
                select id, name, description, row_number() over (order by description)
                from product_type
            ),
            data(id, satellite_id, product,product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
            SELECT
                P.id,
                P.satellite_id,
                P.name,
                PT.name,
                PT.description,
                PR.name,
                S.name,
                P.full_path,
                P.quicklook_image,
                P.footprint,
                P.created_timestamp,
                S.geog
            FROM product P
                JOIN product_type_names PT ON P.product_type_id = PT.id
                JOIN processor PR ON P.processor_id = PR.id
                JOIN site_names S ON P.site_id = S.id
            WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
    $sql$;
    IF $1 IS NOT NULL THEN
        q := q || 'AND P.site_id = $1';

        summer_season_start_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.summer-season.start');
        summer_season_end_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.summer-season.end');
        winter_season_start_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.winter-season.start');
        winter_season_end_str := (select config.value from config where config.site_id = _site_id and "key" = 'downloader.winter-season.end');

        if length(summer_season_start_str) = 8 and length(summer_season_end_str) = 8 then
            summer_season_start_str := right(summer_season_start_str, 4) || left(summer_season_start_str, 4);
            summer_season_end_str := right(summer_season_end_str, 4) || left(summer_season_end_str, 4);
            summer_season_start := summer_season_start_str :: date;
            summer_season_end := summer_season_end_str :: date;
        end if;

        if length(winter_season_start_str) = 8 and length(winter_season_end_str) = 8 then
            winter_season_start_str := right(winter_season_start_str, 4) || left(winter_season_start_str, 4);
            winter_season_end_str := right(winter_season_end_str, 4) || left(winter_season_end_str, 4);
            winter_season_start := winter_season_start_str :: date;
            winter_season_end := winter_season_end_str :: date;
        end if;

        if summer_season_start is not null and summer_season_end is not null or
           winter_season_start is not null and winter_season_end is not null then
            q := q || 'AND (FALSE';
            if summer_season_start is not null and summer_season_end is not null then
                q := q || ' OR P.created_timestamp BETWEEN $3 AND $4';
            end if;
            if winter_season_start is not null and winter_season_end is not null then
                q := q || ' OR P.created_timestamp BETWEEN $5 AND $6';
            end if;
            q := q || ')';
        end if;
    END IF;
    IF $2 IS NOT NULL THEN
        q := q || 'AND P.product_type_id = $2';
    END IF;

    q := q || $sql$
            ORDER BY S.row, PT.row, P.name
        )
--         select * from data;
        SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
    $sql$;

--     raise notice '%', q;

    RETURN QUERY
    EXECUTE q
    USING _site_id, _processor_id, summer_season_start, summer_season_end, winter_season_start, winter_season_end;
END
$BODY$
  LANGUAGE plpgsql STABLE
  COST 100;
ALTER FUNCTION sp_get_dashboard_products(smallint, smallint)
  OWNER TO admin;
