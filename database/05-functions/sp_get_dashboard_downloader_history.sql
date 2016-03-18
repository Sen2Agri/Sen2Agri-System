CREATE OR REPLACE FUNCTION sp_get_dashboard_downloader_history(IN _siteid smallint DEFAULT NULL::smallint)
  RETURNS TABLE(id integer, nr_downloads bigint) AS
$BODY$
BEGIN
   RETURN QUERY
SELECT  case status_id
	when '1' then 1
	when '2' then 2
	when '3' then 3
	when '4' then 3
	when '5' then 2
	end as in_progress_downloads, count(status_id)
FROM downloader_history
WHERE status_id  in ('1','2','3','4','5') AND ( $1 IS NULL OR site_id = _siteid)
group by in_progress_downloads;

END
$BODY$
  LANGUAGE plpgsql

