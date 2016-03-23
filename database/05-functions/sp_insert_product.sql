CREATE OR REPLACE FUNCTION sp_insert_product(_product_type_id smallint, _processor_id smallint, _satellite_id integer, _site_id smallint, _job_id integer, _full_path character varying, _created_timestamp timestamp with time zone, _name character varying, _quicklook_image character varying, _footprint geography, _tiles json)
  RETURNS integer AS
$BODY$
DECLARE return_id product.id%TYPE;
BEGIN
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
        tiles
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
        array(select tile :: character varying from json_array_elements_text(_tiles) tile)
    )
    RETURNING id INTO return_id;

	RETURN return_id;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
