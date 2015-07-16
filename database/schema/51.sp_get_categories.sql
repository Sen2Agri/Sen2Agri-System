CREATE OR REPLACE FUNCTION sp_get_categories()
RETURNS TABLE (
    id config_category.id%TYPE,
    name config_category.name%TYPE,
    allow_per_site_customization config_category.allow_per_site_customization%TYPE
)
AS $$
BEGIN
    RETURN QUERY
        SELECT config_category.id,
               config_category.name,
               config_category.allow_per_site_customization
        FROM config_category
        ORDER BY config_category.display_order;
END
$$
LANGUAGE plpgsql
STABLE;
