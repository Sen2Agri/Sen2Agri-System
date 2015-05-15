CREATE OR REPLACE FUNCTION sp_get_categories() RETURNS 
TABLE (id INT, name CHARACTER VARYING) AS $$
BEGIN

RETURN QUERY SELECT config_category.id, config_category.name FROM config_category ORDER BY config_category.display_order;

END;
$$ LANGUAGE plpgsql;