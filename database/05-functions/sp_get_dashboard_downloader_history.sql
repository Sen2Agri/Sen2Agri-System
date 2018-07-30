CREATE OR REPLACE FUNCTION sp_get_dashboard_downloader_history(IN _siteid smallint DEFAULT NULL::smallint)
  RETURNS TABLE(id integer, nr_downloads bigint, nr_downloads_percentage numeric) AS
$BODY$
DECLARE total numeric;

                BEGIN

		Select count(dh.id) into total from downloader_history dh WHERE status_id IN (1, 2, 3, 4, 5, 6, 7 ,41) AND (_siteid IS NULL OR site_id = _siteid);

                   IF total<>'0' THEN
			   RETURN QUERY 

			    SELECT  1, count(status_id),CASE WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
				ELSE 0 END
				
				FROM downloader_history
				WHERE status_id  = 1 AND ( $1 IS NULL OR site_id = _siteid)
			   UNION
			    SELECT  2, count(status_id),CASE  WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
				ELSE 0 END
				FROM downloader_history
				WHERE status_id IN (2, 5, 6, 7) AND ( $1 IS NULL OR site_id = _siteid)
			   UNION
			    SELECT  3, count(status_id),CASE  WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
				ELSE 0 END
				FROM downloader_history
				WHERE status_id IN (4,41) AND ( $1 IS NULL OR site_id = _siteid)
			    UNION
			    SELECT  4, count(status_id),CASE  WHEN total<>'0' THEN round(((count(status_id) * 100)/total)::numeric ,2)
				ELSE 0 END
				FROM downloader_history
				WHERE status_id IN (3) AND ( $1 IS NULL OR site_id = _siteid)
			   ORDER BY 1;
		    
		   END IF;
                END
                $BODY$
  LANGUAGE plpgsql STABLE
