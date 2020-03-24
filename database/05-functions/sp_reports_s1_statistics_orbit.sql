CREATE OR REPLACE FUNCTION reports.sp_reports_s1_statistics_orbit(
	IN siteId smallint DEFAULT NULL::smallint,
	IN orbitId integer DEFAULT NULL::integer,
	IN fromDate date DEFAULT NULL::date,
	IN toDate date DEFAULT NULL::date)
  RETURNS TABLE(calendar_date date, orbit integer, acquisitions integer) AS
$BODY$
DECLARE startDate date;
DECLARE endDate date;
BEGIN
	IF $3 IS NULL THEN
		SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-01-01')::date INTO startDate FROM reports.s1_report;
	ELSE
		SELECT fromDate INTO startDate;
	END IF;
	IF $4 IS NULL THEN
		SELECT CONCAT(EXTRACT(YEAR FROM CURRENT_DATE)::text,'-12-31') INTO endDate FROM reports.s1_report;
	ELSE
		SELECT toDate INTO endDate;
	END IF;
	RETURN QUERY
	WITH 	calendar AS 
			(SELECT date_trunc('day', dd)::date AS cdate 
				FROM generate_series(startDate::timestamp, endDate::timestamp, '1 day'::interval) dd),
		ac AS 
			(SELECT acquisition_date, orbit_id, COUNT(DISTINCT downloader_history_id) AS acquisitions 
				FROM reports.s1_report 
				WHERE ($1 IS NULL OR site_id = $1) AND ($2 IS NULL OR orbit_id = $2) AND acquisition_date BETWEEN startDate AND endDate
				GROUP BY acquisition_date, orbit_id
				ORDER BY acquisition_date)
	SELECT 	c.cdate,
		COALESCE($1, ac.orbit_id, 0)::integer,
		COALESCE(ac.acquisitions, 0)::integer
	FROM calendar c
		LEFT JOIN ac ON ac.acquisition_date = c.cdate;
END

$BODY$
  LANGUAGE plpgsql STABLE
  COST 100
  ROWS 1000;
ALTER FUNCTION reports.sp_reports_s1_statistics_orbit(smallint, integer, date, date)
  OWNER TO postgres;
