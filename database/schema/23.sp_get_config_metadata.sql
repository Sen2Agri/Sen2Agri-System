CREATE OR REPLACE FUNCTION sp_get_config_metadata()
RETURNS TABLE (
    "key" config_metadata."key"%TYPE,
    friendly_name config_metadata.friendly_name%TYPE,
    "type" config_metadata."type"%TYPE,
    is_advanced config_metadata.is_advanced%TYPE,
    config_category_id config_metadata.config_category_id%TYPE
)
AS $$
BEGIN
    RETURN QUERY
        SELECT config_metadata.key,
               config_metadata.friendly_name,
               config_metadata.type,
               config_metadata.is_advanced,
               config_metadata.config_category_id
        FROM config_metadata
        ORDER BY config_metadata.is_advanced, config_metadata.key;
END
$$
LANGUAGE plpgsql
VOLATILE;
