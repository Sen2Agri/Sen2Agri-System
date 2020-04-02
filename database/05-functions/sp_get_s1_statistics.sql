CREATE OR REPLACE FUNCTION reports.sp_get_s1_statistics(IN site_id smallint)
  RETURNS TABLE("site" smallint, "downloader_history_id" integer, "orbit_id" integer, "acquisition_date" date, "acquisition" character varying, "acquisition_status" character varying, 
		"intersection_date" date, "intersected_product" character varying, "intersected_status" smallint, "intersection" double precision,
		"polarisation" character varying, l2_product character varying, l2_coverage double precision, status_reason character varying) AS
$BODY$

BEGIN
	RETURN QUERY
    WITH d AS (select dh.*,ds.status_description from public.downloader_history dh join public.downloader_status ds on ds.id = dh.status_id)
select 	$1 as site,
	d.id,
	d.orbit_id as orbit, 
 	to_date(substr(split_part(d.product_name, '_', 6), 1, 8),'YYYYMMDD') as acquisition_date, 
	d.product_name as acquisition,
 	d.status_description as acquisition_status,
 	to_date(substr(split_part(i.product_name, '_', 6), 1, 8),'YYYYMMDD') as intersection_date,
 	i.product_name as intersected_product,
 	i.status_id as intersected_status,
 	st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) * 100 as intersection,
 	split_part(p.name, '_', 6)::character varying as polarisation,
 	p.name as l2_product,
 	st_area(st_intersection(d.footprint, p.geog))/st_area(d.footprint) * 100 as l2_coverage,
 	d.status_reason
	from d
	join public.downloader_history i on i.site_id = d.site_id AND i.orbit_id = d.orbit_id AND i.satellite_id = d.satellite_id and st_intersects(d.footprint, i.footprint) AND DATE_PART('day', d.product_date - i.product_date) BETWEEN 5 AND 7 AND st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) > 0.05
	join public.product p on p.downloader_history_id = d.id
	WHERE NOT EXISTS(SELECT sr.* FROM reports.s1_report sr WHERE sr.downloader_history_id = d.id AND sr.intersected_product = i.product_name AND sr.site_id = i.site_id AND sr.l2_product = p.name)
		and d.site_id = $1 AND d.satellite_id = 3 and i.id is not null
		and p.name like concat('%', substr(split_part(i.product_name, '_', 6), 1, 15),'%')
union
select 	$1 as site,
	d.id,
	d.orbit_id as orbit, 
 	to_date(substr(split_part(d.product_name, '_', 6), 1, 8),'YYYYMMDD') as acquisition_date, 
	d.product_name as acquisition,
 	d.status_description as acquisition_status,
 	to_date(substr(split_part(i.product_name, '_', 6), 1, 8),'YYYYMMDD') as intersection_date,
 	i.product_name as intersected_product,
 	i.status_id as intersected_status,
 	st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) * 100 as intersection,
 	split_part(p.name, '_', 6)::character varying as polarisation,
 	p.name as l2_product,
 	st_area(st_intersection(d.footprint, p.geog))/st_area(d.footprint) * 100 as l2_coverage,
 	d.status_reason
	from d
	join public.downloader_history i on i.site_id = d.site_id AND i.orbit_id = d.orbit_id AND i.satellite_id = d.satellite_id and st_intersects(d.footprint, i.footprint) AND DATE_PART('day', d.product_date - i.product_date) BETWEEN 11 AND 13 AND st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) > 0.05
	join public.product p on p.downloader_history_id = d.id
	left outer join public.product_stats ps on ps.product_id = p.id
	WHERE NOT EXISTS(SELECT sr.* FROM reports.s1_report sr WHERE sr.downloader_history_id = d.id AND sr.intersected_product = i.product_name AND sr.site_id = i.site_id AND sr.l2_product = p.name)
		and d.site_id = $1 AND d.satellite_id = 3 and i.id is not null and left(d.product_name, 3) = left(i.product_name, 3)
		and p.name like concat('%', substr(split_part(i.product_name, '_', 6), 1, 15),'%')
union
select 	$1 as site,
	d.id,
	d.orbit_id as orbit,
	to_date(substr(split_part(d.product_name, '_', 6), 1, 8),'YYYYMMDD') as acquisition_date, 
	d.product_name as acquisition,
 	ds.status_description as acquisition_status,
 	to_date(substr(split_part(i.product_name, '_', 6), 1, 8),'YYYYMMDD') as intersection_date,
 	i.product_name as intersected_product,
 	i.status_id as intersected_status,
 	case when i.footprint is null then null else st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) * 100 end as intersection,
 	null as polarisation,
 	null as l2_product,
 	null as l2_coverage,
 	null as status_reason
	from public.downloader_history d
		join public.downloader_status ds on ds.id = d.status_id
		left outer join public.downloader_history i on i.site_id = d.site_id AND i.orbit_id = d.orbit_id AND i.satellite_id = d.satellite_id and st_intersects(d.footprint, i.footprint) AND DATE_PART('day', d.product_date - i.product_date) BETWEEN 5 AND 7 AND st_area(st_intersection(i.footprint, d.footprint)) / st_area(d.footprint) > 0.05
	where NOT EXISTS(SELECT sr.* FROM reports.s1_report sr WHERE sr.downloader_history_id = d.id) and d.site_id = $1 AND d.satellite_id = 3 and d.status_id != 5;
END

$BODY$
  LANGUAGE plpgsql STABLE
  COST 100
  ROWS 1000;
ALTER FUNCTION reports.sp_get_s1_statistics(smallint)
  OWNER TO postgres;
