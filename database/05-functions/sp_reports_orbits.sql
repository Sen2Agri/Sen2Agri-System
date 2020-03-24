CREATE OR REPLACE FUNCTION reports.sp_reports_orbits(
	IN _satellite_id smallint,
	IN _site_id smallint DEFAULT NULL::smallint)
  RETURNS TABLE(orbit_id integer) AS
$BODY$
BEGIN
	IF $1 = 3 THEN
		RETURN QUERY
		SELECT DISTINCT r.orbit_id::integer FROM reports.s1_report r WHERE ($2 IS NULL OR site_id = $2) ORDER BY r.orbit_id::integer;
	ELSIF $1 = 1 THEN
		RETURN QUERY
		SELECT DISTINCT r.orbit_id::integer FROM reports.s2_report r WHERE ($2 IS NULL OR site_id = $2) ORDER BY r.orbit_id::integer;
	ELSE
		RETURN QUERY
		SELECT DISTINCT r.orbit_id::integer FROM reports.l8_report r WHERE ($2 IS NULL OR site_id = $2) ORDER BY r.orbit_id::integer;
	END IF;
END

$BODY$
  LANGUAGE plpgsql STABLE
  COST 100
  ROWS 1000;
ALTER FUNCTION reports.sp_reports_orbits(smallint, smallint)
  OWNER TO postgres;
