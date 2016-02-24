CREATE OR REPLACE FUNCTION sp_insert_product(
    _product_type_id product.product_type_id%TYPE,
    _processor_id product.processor_id%TYPE,
    _task_id product.task_id%TYPE,
    _full_path product.full_path%TYPE,
    _created_timestamp product.created_timestamp%TYPE
) RETURNS product.id%TYPE
AS $$
DECLARE return_id product.id%TYPE;
BEGIN
    INSERT INTO product(
        product_type_id,
        processor_id,
        task_id,
        site_id,
        full_path,
        created_timestamp
    )
    VALUES (
        _product_type_id,
        _processor_id,
        _task_id,
        (SELECT job.site_id
         FROM task
         INNER JOIN job ON job.id = task.job_id
         WHERE task.id = _task_id),
        _full_path,
        _created_timestamp
    )
    RETURNING id INTO return_id;

	RETURN return_id;
END;
$$
LANGUAGE plpgsql
VOLATILE;
