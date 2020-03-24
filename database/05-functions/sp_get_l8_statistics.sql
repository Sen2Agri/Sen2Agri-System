CREATE OR REPLACE FUNCTION reports.sp_get_l8_statistics(IN site_id smallint)
  RETURNS TABLE("site" smallint, "downloader_history_id" integer, "orbit_id" integer, "acquisition_date" date, "acquisition" character varying, "acquisition_status" character varying, 
		status_reason character varying, l2_product character varying, clouds integer) AS
$BODY$

BEGIN
	RETURN QUERY
    SELECT 	$1 as site_id,
		d.id,
		split_part(d.product_name, '_', 3)::integer as orbit, 
		to_date(split_part(d.product_name, '_', 4), 'YYYYMMDD') as acquisition_date, 
		d.product_name as acquisition, 
		ds.status_description as status, 
		case 	when d.status_id in (1,2,3,4,41,5) then d.status_reason
			when d.status_id in (6,7,8) then concat('clouds:',coalesce(th.cloud_coverage::varchar,'n/a'),'; snow:',coalesce(th.snow_coverage::varchar,'n/a'),'; failure:',coalesce(regexp_replace(th.failed_reason,E'[\\n]+',' ','g'),'n/a'))
			else null end as status_reason, 
		p.name as l2_product, 
		coalesce(th.cloud_coverage, -1) as clouds 
	FROM public.downloader_history d
		JOIN public.downloader_status ds ON ds.id = d.status_id 
		LEFT JOIN public.l1_tile_history th ON th.downloader_history_id = d.id 
		LEFT JOIN public.product p ON REPLACE(p.name, '_L2A_', '_L1TP_') = d.product_name 
	WHERE NOT EXISTS(SELECT sr.* FROM reports.l8_report sr WHERE sr.downloader_history_id = d.id) AND d.satellite_id = 2 and d.site_id = $1
	ORDER BY d.orbit_id, acquisition_date, d.product_name, l2_product;
END

$BODY$
  LANGUAGE plpgsql STABLE
  COST 100
  ROWS 1000;
ALTER FUNCTION reports.sp_get_l8_statistics(smallint)
  OWNER TO postgres;
