-- Function: sp_get_dashboard_products_nodes(character varying, integer[], integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[], boolean)

-- DROP FUNCTION sp_get_dashboard_products_nodes(character varying, integer[], integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[], boolean);

CREATE OR REPLACE FUNCTION sp_get_dashboard_products_nodes(
    _user_name character varying,
    _site_id integer[] DEFAULT NULL::integer[],
    _product_type_id integer[] DEFAULT NULL::integer[],
    _season_id smallint DEFAULT NULL::smallint,
    _satellit_id integer[] DEFAULT NULL::integer[],
    _since_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
    _until_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
    _tiles character varying[] DEFAULT NULL::character varying[],
    _get_nodes boolean DEFAULT false)
  RETURNS SETOF json AS
$BODY$
	DECLARE q text;
	BEGIN
	    q := $sql$
	    WITH
		product_type_names(id, name, description, row, is_raster) AS (
		select id, name, description, row_number() over (order by description), is_raster
		from product_type
		-- LPIS products should be excluded
		where name != 'lpis'
		),
	$sql$;

	IF $9 IS TRUE THEN
		q := q || $sql$
	    site_names(id, name, geog, row) AS (
		select s.id, s.name, st_astext(s.geog), row_number() over (order by s.name)
		from site s
		join public.user u on u.login = $1 and (u.role_id = 1 or s.id in (select * from unnest(u.site_id)))
		),
	    data(id, product, footprint, site_coord, product_type_id, satellite_id, is_raster) AS (

		SELECT
		P.id,
		P.name,
		P.footprint,
		S.geog,
		PT.id,
		P.satellite_id,
		PT.is_raster
		$sql$;
	    ELSE
	    q := q || $sql$
	    site_names(id, name,  row) AS (
		select id, name, row_number() over (order by name)
		from site
		),
	      data(id, satellite_id, product_type_id, product_type_description,site, site_id) AS (
		SELECT
		P.id,
		P.satellite_id,
		PT.id,
		PT.description,
		S.name,
		S.id
	     $sql$;
	END IF;

	 q := q || $sql$
		FROM product P
		JOIN product_type_names PT ON P.product_type_id = PT.id
		JOIN processor PR ON P.processor_id = PR.id
		JOIN site_names S ON P.site_id = S.id
		WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
		AND EXISTS (
		    SELECT * FROM season WHERE season.site_id = P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date + interval '1 day'
	    $sql$;
	    IF $4 IS NOT NULL THEN
	    q := q || $sql$
		AND season.id=$4
		$sql$;
	    END IF;

	    q := q || $sql$
	    )
	    $sql$;
	     raise notice '%', _site_id;raise notice '%', _product_type_id;raise notice '%', _satellit_id;
	    IF $2 IS NOT NULL THEN
	    q := q || $sql$
		AND P.site_id = ANY($2)

	    $sql$;
	    END IF;

	    IF $3 IS NOT NULL THEN
	    q := q || $sql$
		AND P.product_type_id= ANY($3)

		$sql$;
	    END IF;

	IF $6 IS NOT NULL THEN
	q := q || $sql$
	    AND P.created_timestamp >= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS')
	    $sql$;
	END IF;

	IF $7 IS NOT NULL THEN
	q := q || $sql$
	    AND P.created_timestamp <= to_timestamp(cast($7 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day'
	    $sql$;
	END IF;

	IF $8 IS NOT NULL THEN
	q := q || $sql$
	    AND P.tiles <@$8 AND P.tiles!='{}'
	    $sql$;
	END IF;

	q := q || $sql$
	    ORDER BY S.row, PT.row, P.name
	    )
	--         select * from data;
	    SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
	    $sql$;

	    raise notice '%', q;

	    RETURN QUERY
	    EXECUTE q
	    USING _user_name, _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles, _get_nodes;
	END
	$BODY$
  LANGUAGE plpgsql STABLE
  COST 100
  ROWS 1000;
ALTER FUNCTION sp_get_dashboard_products_nodes(character varying, integer[], integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[], boolean)
  OWNER TO admin;
