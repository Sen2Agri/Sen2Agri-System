
CREATE OR REPLACE FUNCTION sp_get_dashboard_processor_scheduled_task(IN _processor_id smallint)
  RETURNS TABLE(id smallint, name character varying, site_name character varying, repeat_type smallint, first_run_time character varying, repeat_after_days smallint, repeat_on_month_day smallint, processor_params character varying) AS
$BODY$
BEGIN
   RETURN QUERY
	SELECT st.id,
              st.name,        
              site.name,
              st.repeat_type,
              st.first_run_time,
              st.repeat_after_days,
              st.repeat_on_month_day,
              st.processor_params
        FROM scheduled_task AS st, site
        WHERE st.processor_id = _processor_id AND st.site_id=site.id
        ORDER BY st.id;
    
        
END
$BODY$
  LANGUAGE plpgsql