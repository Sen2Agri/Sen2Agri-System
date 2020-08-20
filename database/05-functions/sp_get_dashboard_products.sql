CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
    _site_id integer[] DEFAULT NULL::integer[],
    _product_type_id integer[] DEFAULT NULL::integer[],
    _season_id smallint DEFAULT NULL::smallint,
    _satellit_id integer[] DEFAULT NULL::integer[],
    _since_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
    _until_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
    _tiles character varying[] DEFAULT NULL::character varying[])
  RETURNS SETOF json AS
$BODY$
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
			    data(id, satellite_id, product, product_type_id, product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
			    SELECT
				P.id,
				P.satellite_id,
				P.name,
				PT.id,
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
			    AND EXISTS (
				    SELECT * FROM season WHERE season.site_id =P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date + interval '1 day'
		    $sql$;
		    IF $3 IS NOT NULL THEN
			q := q || $sql$
				AND season.id=$3
				$sql$;
		    END IF;

		    q := q || $sql$
			)
		    $sql$;

		    IF $1 IS NOT NULL THEN
			q := q || $sql$
				AND P.site_id = ANY($1)
		    $sql$;
		    END IF;

		    IF $2 IS NOT NULL THEN
			q := q || $sql$
			 	AND P.product_type_id= ANY($2)

				$sql$;
		    END IF;

		IF $5 IS NOT NULL THEN
		q := q || $sql$
			AND P.created_timestamp >= to_timestamp(cast($5 as TEXT),'YYYY-MM-DD HH24:MI:SS')
			$sql$;
		END IF;

		IF $6 IS NOT NULL THEN
		q := q || $sql$
			AND P.created_timestamp <= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day'
			$sql$;
		END IF;

		IF $7 IS NOT NULL THEN
		q := q || $sql$
			AND P.tiles <@$7 AND P.tiles!='{}'
			$sql$;
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
		    USING _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles;
		END
		$BODY$
  LANGUAGE plpgsql STABLE;
