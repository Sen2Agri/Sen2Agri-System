CREATE OR REPLACE FUNCTION sp_insert_product(
    _product_type_id product.product_type_id%TYPE,
    _processor_id product.processor_id%TYPE,
    _satellite_id satellite.id%TYPE,
    _site_id site.id%TYPE,
    _job_id job.id%TYPE,
    _full_path product.full_path%TYPE,
    _created_timestamp product.created_timestamp%TYPE,
    _name product.name%TYPE,
    _quicklook_image product.quicklook_image%TYPE,
    _footprint GEOGRAPHY
) RETURNS product.id%TYPE
AS $$
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
        geog
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
         _footprint
    )
    RETURNING id INTO return_id;

	RETURN return_id;
END;
$$
LANGUAGE plpgsql
VOLATILE;
