CREATE OR REPLACE FUNCTION sp_insert_product(
    _product_type_id smallint,
    _processor_id smallint,
    _satellite_id integer,
    _site_id smallint,
    _job_id integer,
    _full_path character varying,
    _created_timestamp timestamp with time zone,
    _name character varying,
    _quicklook_image character varying,
    _footprint geography,
    _orbit_id integer,
    _tiles json,
    _orbit_type_id smallint DEFAULT NULL::smallint,
    _downloader_history_id integer DEFAULT NULL::integer)
  RETURNS integer AS
$BODY$
DECLARE return_id product.id%TYPE;
BEGIN
    UPDATE product
    SET job_id = _job_id,
        full_path = _full_path,
        created_timestamp = _created_timestamp,
        quicklook_image = _quicklook_image,
        footprint = (SELECT '(' || string_agg(REPLACE(replace(ST_AsText(geom) :: text, 'POINT', ''), ' ', ','), ',') || ')'
                     from ST_DumpPoints(ST_Envelope(_footprint :: geometry))
                     WHERE path[2] IN (1, 3)) :: POLYGON,
        geog = _footprint,
        tiles = array(select tile :: character varying from json_array_elements_text(_tiles) tile),
        is_archived = FALSE
    WHERE product_type_id = _product_type_id
      AND processor_id = _processor_id
      AND satellite_id = _satellite_id
      AND site_id = _site_id
      AND COALESCE(orbit_id, 0) = COALESCE(_orbit_id, 0)
      AND "name" = _name
    RETURNING id INTO return_id;

    IF NOT FOUND THEN
        INSERT INTO product(
            product_type_id,
            processor_id,
            satellite_id,
            job_id,
            site_id,
            full_path,
            created_timestamp,
            "name",
            quicklook_image,
            footprint,
            geog,
            orbit_id,
            tiles,
            orbit_type_id,
            downloader_history_id
        )
        VALUES (
            _product_type_id,
            _processor_id,
            _satellite_id,
            _job_id,
            _site_id,
            _full_path,
            COALESCE(_created_timestamp, now()),
            _name,
            _quicklook_image,
            (SELECT '(' || string_agg(REPLACE(replace(ST_AsText(geom) :: text, 'POINT', ''), ' ', ','), ',') || ')'
             from ST_DumpPoints(ST_Envelope(_footprint :: geometry))
             WHERE path[2] IN (1, 3)) :: POLYGON,
             _footprint,
             _orbit_id,
            array(select tile :: character varying from json_array_elements_text(_tiles) tile),
            _orbit_type_id,
            _downloader_history_id
        )
        RETURNING id INTO return_id;
        
        INSERT INTO event(
            type_id,
            data,
            submitted_timestamp)
            VALUES (
            3, -- "ProductAvailable"
            ('{"product_id":' || return_id || '}') :: json,
            now()
        );
    END IF;

	RETURN return_id;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE;
ALTER FUNCTION sp_insert_product(smallint, smallint, integer, smallint, integer, character varying, timestamp with time zone, character varying, character varying, geography, integer, json, smallint, integer)
  OWNER TO admin;

