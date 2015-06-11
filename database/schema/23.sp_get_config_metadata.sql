CREATE OR REPLACE FUNCTION sp_get_config_metadata(_is_admin BOOLEAN)
RETURNS TABLE (
    "key" config_metadata."key"%TYPE,
    friendly_name config_metadata.friendly_name%TYPE,
    "type" config_metadata."type"%TYPE,
    is_advanced config_metadata.is_advanced%TYPE,
    config_category_id config_metadata.config_category_id%TYPE
)
AS $$
BEGIN
    if _is_admin THEN
        RETURN QUERY
            SELECT config_metadata.key,
                   config_metadata.friendly_name,
                   config_metadata.type,
                   config_metadata.is_advanced,
                   config_metadata.config_category_id
            FROM config_metadata
            ORDER BY config_metadata.is_advanced, config_metadata.key;
    ELSE
        RETURN QUERY
            SELECT config_metadata.key,
                   config_metadata.friendly_name,
                   config_metadata.type,
                   config_metadata.is_advanced,
                   config_metadata.config_category_id
            FROM config_metadata
            WHERE not config_metadata.is_advanced
            ORDER BY config_metadata.is_advanced, config_metadata.key;
    END IF;
END
$$
LANGUAGE plpgsql
VOLATILE;
