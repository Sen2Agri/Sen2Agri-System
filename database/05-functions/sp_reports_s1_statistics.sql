CREATE OR REPLACE FUNCTION reports.sp_reports_s1_statistics(
	IN siteId smallint DEFAULT NULL::smallint,
	IN orbitId integer DEFAULT NULL::integer,
	IN fromDate date DEFAULT NULL::date,
	IN toDate date DEFAULT NULL::date)
  RETURNS TABLE(calendar_date date, acquisitions integer, failed_to_download integer, pairs integer, 
		processed integer, not_yet_processed integer, falsely_processed integer, no_intersections integer, errors integer) AS
$BODY$
DECLARE startDate date;
DECLARE endDate date;
BEGIN
	IF $3 IS NULL THEN
		SELECT MIN(acquisition_date) INTO startDate FROM reports.s1_report;
	ELSE
		SELECT fromDate INTO startDate;
	END IF;
	IF $4 IS NULL THEN
		SELECT MAX(acquisition_date) INTO endDate FROM reports.s1_report;
	ELSE
		SELECT toDate INTO endDate;
	END IF;
	RETURN QUERY
	WITH 	calendar AS 
			(SELECT date_trunc('day', dd)::date AS cdate 
				FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
		ac AS 
			(SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS acquisitions 
				FROM reports.s1_report 
				WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date 
				ORDER BY acquisition_date),
		p AS 
			(SELECT acquisition_date, COUNT(intersected_product) AS pairs 
				FROM reports.s1_report 
				WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date 
				ORDER BY acquisition_date),
		proc AS 
			(SELECT acquisition_date, COUNT(intersected_product) AS cnt 
				FROM reports.s1_report 
				WHERE status_description = 'processed' AND
					($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date 
				ORDER BY acquisition_date),
		ndld AS 
			(SELECT acquisition_date, count(downloader_history_id) AS cnt 
				FROM reports.s1_report 
				WHERE status_description IN ('failed','aborted') AND intersected_product IS NULL AND
					($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date 
				ORDER BY acquisition_date),
        dld AS
            (SELECT r.acquisition_date, COUNT(r.downloader_history_id) AS cnt
                FROM reports.s1_report r
                WHERE r.status_description IN ('downloaded', 'processing') AND r.intersected_product IS NOT NULL AND
                    ($1 IS NULL OR r.site_id = $1) AND ($2 IS NULL OR r.orbit_id = $2) AND r.acquisition_date BETWEEN startDate AND endDate
                     AND NOT EXISTS (SELECT s.downloader_history_id FROM reports.s1_report s
                                    WHERE s.downloader_history_id = r.downloader_history_id AND r.l2_product LIKE '%COHE%')
                GROUP BY acquisition_date
                ORDER BY acquisition_date),
		fproc AS 
			(SELECT acquisition_date, COUNT(DISTINCT downloader_history_id) AS cnt 
				FROM reports.s1_report 
				WHERE status_description = 'processed' AND intersected_product IS NULL AND
					($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date 
				ORDER BY acquisition_date),
		ni AS 
			(SELECT acquisition_date, count(downloader_history_id) AS cnt 
				FROM reports.s1_report 
				WHERE status_description = 'downloaded' AND intersected_product IS NULL AND
					($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date 
				ORDER BY acquisition_date),
		e AS 
			(SELECT acquisition_date, COUNT(intersected_product) AS cnt 
				FROM reports.s1_report 
				WHERE status_description LIKE 'processing_%failed' AND
					($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date 
				ORDER BY acquisition_date)
	SELECT 	c.cdate, 
		COALESCE(ac.acquisitions, 0)::integer,
		COALESCE(ndld.cnt, 0)::integer,
		COALESCE(p.pairs, 0)::integer, 
		COALESCE(proc.cnt, 0)::integer, 
		COALESCE(dld.cnt, 0)::integer,
		COALESCE(fproc.cnt, 0)::integer,
		COALESCE(ni.cnt, 0)::integer,
		COALESCE(e.cnt, 0)::integer
	FROM calendar c
		LEFT JOIN ac ON ac.acquisition_date = c.cdate
		LEFT JOIN ndld ON ndld.acquisition_date = c.cdate
		LEFT JOIN p ON p.acquisition_date = c.cdate
		LEFT JOIN proc ON proc.acquisition_date = c.cdate
		LEFT JOIN dld ON dld.acquisition_date = c.cdate
		LEFT JOIN fproc ON fproc.acquisition_date = c.cdate
		LEFT JOIN ni ON ni.acquisition_date = c.cdate
		LEFT JOIN e ON e.acquisition_date = c.cdate;
END

$BODY$
  LANGUAGE plpgsql STABLE
  COST 100
  ROWS 1000;
ALTER FUNCTION reports.sp_reports_s1_statistics(smallint, integer, date, date)
  OWNER TO postgres;
