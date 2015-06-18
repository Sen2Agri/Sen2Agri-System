CREATE OR REPLACE FUNCTION sp_get_categories()
RETURNS TABLE (
    id config_category.id%TYPE,
    "name" config_category.name%TYPE
)
AS $$
BEGIN
    RETURN QUERY
        SELECT config_category.id,
               config_categor.name
        FROM config_category
        ORDER BY config_category.display_order;
END
$$
LANGUAGE plpgsql
VOLATILE;
