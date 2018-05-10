CREATE OR REPLACE FUNCTION sp_get_dashboard_downloader_history(IN _siteid smallint DEFAULT NULL::smallint)
  RETURNS TABLE(id integer, nr_downloads bigint, nr_downloads_percentage numeric) AS
$BODY$
DECLARE total numeric;
DECLARE t1 numeric DEFAULT 0;
DECLARE t2 numeric  DEFAULT 0;
                BEGIN

                SELECT COALESCE(SUM(product_count),0)  into t1 
                FROM downloader_count
		WHERE (_siteid IS NULL OR site_id = _siteid) 
		     AND (site_id,last_updated) IN 
			 (SELECT site_id,max(last_updated) 
			  FROM downloader_count 
			  WHERE (_siteid IS NULL OR site_id = _siteid) AND satellite_id=1 AND product_count>=0 GROUP BY site_id 
			  );	

                --RAISE NOTICE '%',t1; 
                
		SELECT COALESCE(SUM(product_count),0)  into t2 
                FROM downloader_count
		WHERE (_siteid IS NULL OR site_id = _siteid) 
		AND (site_id,last_updated) IN (
			SELECT site_id,max(last_updated) FROM downloader_count 
			WHERE (_siteid IS NULL OR site_id = _siteid) AND satellite_id=2 AND product_count>=0  GROUP BY site_id 
			);	

                --RAISE NOTICE '%',t2;
                      
		SELECT COALESCE(t1+t2,0) into total;  

                
                   RAISE NOTICE '%',total;     
		   -- if total not 0 => still downloading
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
				WHERE status_id IN (3, 4) AND ( $1 IS NULL OR site_id = _siteid)
			   ORDER BY 1;
		    -- ELSE if the total is 0 => no products to download
		    
		   END IF;
                END
                $BODY$
  LANGUAGE plpgsql STABLE
