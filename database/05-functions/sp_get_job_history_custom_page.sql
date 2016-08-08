CREATE OR REPLACE FUNCTION sp_get_job_history_custom_page(
    IN _siteid smallint DEFAULT NULL::smallint,
    IN _page integer DEFAULT 1,
    IN _rows_per_page integer DEFAULT 20)
  RETURNS TABLE(id integer, end_timestamp timestamp with time zone, processor character varying, site character varying, status character varying, start_type character varying) AS
$BODY$
BEGIN
    RETURN QUERY
        SELECT J.id, J.end_timestamp, P.name, S.name, AST.name, ST.name
		FROM job J
			JOIN processor P ON J.processor_id = P.id
			JOIN site S ON J.site_id = S.id
			JOIN job_start_type ST ON J.start_type_id = ST.id
			JOIN activity_status AST ON J.status_id = AST.id
		WHERE   $1 IS NULL OR site_id = _siteid
	ORDER BY J.end_timestamp DESC
	OFFSET ($2 - 1) * _rows_per_page LIMIT _rows_per_page;
END
$BODY$
  LANGUAGE plpgsql STABLE
  COST 100
  ROWS 1000;
ALTER FUNCTION sp_get_job_history_custom_page(smallint, integer, integer)
  OWNER TO admin;
