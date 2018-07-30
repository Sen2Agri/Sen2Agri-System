CREATE OR REPLACE FUNCTION sp_get_dashboard_products_nodes(
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
			    product_type_names(id, name, description, row) AS (
				select id, name, description, row_number() over (order by description)
				from product_type
			    ),
		$sql$;
		
		IF $8 IS TRUE THEN
	            q := q || $sql$
			site_names(id, name, geog, row) AS (
				select id, name, st_astext(geog), row_number() over (order by name)
				from site
			    ),
			data(id, product, footprint, site_coord, product_type_id, satellite_id ) AS (
			    
			    SELECT
				P.id,
				P.name,
				P.footprint,
				S.geog,
				PT.id,
				P.satellite_id
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
				    SELECT * FROM season WHERE season.site_id = P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date 
		    $sql$;
		    IF $3 IS NOT NULL THEN
			q := q || $sql$
				AND season.id=$3			       
				$sql$;
		    END IF;
				
		    q := q || $sql$
			)
		    $sql$;
		     raise notice '%', _site_id;raise notice '%', _product_type_id;raise notice '%', _satellit_id;
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

		/*IF $4 IS NOT NULL THEN
		q := q || $sql$
			AND P.satellite_id= ANY($4)

			$sql$;
		END IF;*/
	
		
		q := q || $sql$
			ORDER BY S.row, PT.row, P.name
			)
		--         select * from data;
			SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
		    $sql$;	

		    raise notice '%', q;

		    RETURN QUERY
		    EXECUTE q
		    USING _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles, _get_nodes;
		END
		$BODY$
  LANGUAGE plpgsql STABLE
