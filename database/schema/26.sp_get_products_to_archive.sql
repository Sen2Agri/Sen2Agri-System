CREATE OR REPLACE FUNCTION sp_get_products_to_archive() RETURNS 
TABLE (product_id int, current_path varchar, archive_path varchar) AS $$
BEGIN

	CREATE TEMP TABLE settings (
		processor_id smallint,
		product_type_id smallint,
		site_id smallint,
		age smallint,
		archive_path varchar
		) ON COMMIT DROP;

	-- Get the configured 2 main archiver settings (max_age and archive_path), split by site, processor and product
	WITH 
	t1 AS
	(SELECT processor.id as processor_id, product_type.id as product_type_id, params.site_id, cast(params.value as int) as age
		FROM sp_get_parameters('archiver') AS params 
		LEFT OUTER JOIN processor ON params.key ILIKE '%' || processor.short_name || '%' 
		LEFT OUTER JOIN product_type ON params.key ILIKE '%' || product_type.name || '%' 
		WHERE params.key ILIKE '%max_age%'), 
	t2 AS 
	(SELECT processor.id as processor_id, product_type.id as product_type_id, params.site_id, params.value as archive_path
		FROM sp_get_parameters('archiver') AS params 
		LEFT OUTER JOIN processor ON params.key ILIKE '%' || processor.short_name || '%' 
		LEFT OUTER JOIN product_type ON params.key ILIKE '%' || product_type.name || '%' 
		WHERE params.key ILIKE '%archive_path%')
	INSERT INTO settings (
		processor_id,
		product_type_id,
		site_id,
		age,
		archive_path)
	SELECT coalesce(t1.processor_id, t2.processor_id) as processor_id,
		coalesce(t1.product_type_id, t2.product_type_id) as product_type_id,
		coalesce(t1.site_id, t2.site_id) as site_id,
		t1.age,
		t2.archive_path
	FROM t1 INNER JOIN t2
	ON (t1.processor_id = t2.processor_id OR t1.processor_id IS NULL OR t2.processor_id IS NULL)
	AND (t1.product_type_id = t2.product_type_id OR t1.product_type_id IS NULL OR t2.product_type_id IS NULL)
	AND (t1.site_id = t2.site_id OR t1.site_id IS NULL OR t2.site_id IS NULL);

	-- Get the products that have not been archived, determine their new path and return them
	RETURN QUERY
	WITH unarchived_products AS
	(SELECT * FROM product WHERE is_archived = false ORDER BY full_path)
	SELECT 
	unarchived_products.id AS product_id,
	regexp_replace(unarchived_products.full_path, '[/\\]{2,}', '/')::varchar AS current_path,
	sp_build_archive_path(settings.archive_path, site.short_name, processor.short_name, product_type.name, unarchived_products.full_path) AS archive_path
	FROM unarchived_products INNER JOIN settings
	ON (unarchived_products.processor_id = settings.processor_id OR settings.processor_id IS NULL)
	AND (unarchived_products.product_type_id = settings.product_type_id OR settings.product_type_id IS NULL)
	AND (unarchived_products.site_id = settings.site_id OR settings.site_id IS NULL)
	INNER JOIN processor ON unarchived_products.processor_id = processor.id
	INNER JOIN product_type ON unarchived_products.product_type_id = product_type.id
	INNER JOIN site ON unarchived_products.site_id = site.id
	ORDER BY unarchived_products.created;

END;
$$ LANGUAGE plpgsql;