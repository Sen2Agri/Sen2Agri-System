CREATE OR REPLACE VIEW v_get_sites
AS
SELECT site.id,
       site.name
FROM site
ORDER BY site.name;
