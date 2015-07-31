CREATE OR REPLACE FUNCTION sp_upsert_parameter(
    _key config_metadata.key%TYPE,
    _friendly_name config_metadata.friendly_name%TYPE,
    _data_type config_metadata.type%TYPE,
    _is_advanced config_metadata.is_advanced%TYPE,
    _config_category config_category.name%TYPE,
    _default_value config.value%TYPE
)
RETURNS VOID
AS $$
-- SELECT sp_upsert_parameter('http-listener.root-path', 'Document Root Path', 'directory', TRUE, 'Dashboard', '/srv/sen2agri-dashboard');
DECLARE category_id config_category.id%TYPE;
BEGIN
    SELECT config_category.id
    FROM config_category
    WHERE config_category.name = _config_category
    INTO category_id;

    IF category_id IS NULL THEN
        INSERT INTO config_category("name", display_order)
        VALUES(_config_category, (SELECT MAX(display_order) + 1
                                  FROM config_category))
        RETURNING config_category.id
        INTO category_id;
    END IF;

    PERFORM sp_upsert_parameters(array_to_json(array_agg(row_to_json(t))), TRUE)
    FROM (SELECT _key AS "key",
                 NULL AS site_id,
                 _friendly_name AS friendly_name,
                 _default_value AS "value",
                 _data_type AS "type",
                 _is_advanced AS is_advanced,
                 category_id AS config_category_id) t;
END;
$$ language plpgsql;
