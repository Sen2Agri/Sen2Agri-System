<?php 

function update_scheduled_jobs_layout($processor_id)
{
	// Remove the old divs

	//$counter = 0; // DO NOT REMOVE - This is used when updating.
	$db = pg_connect ( 'host=sen2agri-dev port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );

	/*schedule type
	 * once = 0,
	 * cycle = 1,
	 * repeat=2
	 * */
	
	$action = "dashboard.php";
/*	if($processor_id == '1'){
		$action = "dashboard.php#tab_l2a";
	}elseif ($processor_id == '2'){
		$action = "dashboard.php#tab_l3a";
	}elseif ($processor_id == '3'){
		$action = "dashboard.php#tab_l3b";
	}elseif ($processor_id == '4'){
		$action = "dashboard.php#tab_l4a";
	}elseif ($processor_id == '5'){
		$action = "dashboard.php#tab_l4b";
				}*/
	
	$sql =" SELECT st.id,
			st.name,
			site.name,
			st.repeat_type,
			st.first_run_time,
			st.repeat_after_days,
			st.repeat_on_month_day 
			FROM scheduled_task as st,site
   			WHERE st.processor_id=$processor_id AND st.site_id=site.id";
	
	/*$sql =" SELECT st.name,
	site.name,
	st.repeat_type,
	st.first_run_time,
	st.repeat_after_days,
	st.repeat_on_month_day
	FROM scheduled_task as st,site
	WHERE st.site_id=site.id";*/
	
	
	$result = pg_query($db,$sql) or die("Could not execute.");
	while ($row = pg_fetch_row($result)){
		

		// Set up the DOM elements
		$div = "<div class=\"panel_job_container\" id=\"pnl_l2a_job_container\">".
		"<div class=\"panel panel-default panel_job\" id=\"pnl_l2a_job\">".
		"<div class=\"panel-heading\"  id=\"job_name\"> ".$row[1]."</div>".
		"<form role=\"form\" id=\"l2aform\" name=\"l2aform\" method=\"post\" action=\"".$action."\" style=\"padding:10px;\">".
		"<input type=\"hidden\" value=\"".$row[0]."\" name=\"scheduledID\">".
		"<span class=\"form-group form-group-sm div-scheduled\">".
		"<label class=\"control-label\" for=\"siteName\">Site: <span class=\"schedule_format\">".$row[2]."</span></label>".
		"</span>".
		" <span class=\"form-group form-group-sm div-scheduled\">".
		"<label class=\"control-label\" for=\"schedule\"> Schedule:</label> " .
		"<span class=\"schedule_format\">".
		"<select id=\"schedule\" name=\"schedule\" onchange=\"selectedSchedule()\">".
		"<option value=\"0\" .(($row[3] == 0) ? \" selected\" : \"\").>Once</option>".
		"<option value=\"1\" .(($row[3] == 1) ? \" selected\" : \"\").>Cycle</option>".
		"<option value=\"2\" .(($row[3] == 2) ? \" selected\" : \"\").>Repeat</option>".
		"</select>".
		"</span>".
		"</span>".
		"<span class=\"form-group form-group-sm div-scheduled\" id=\"div_startdate\">".
		"<label class=\"control-label\" for=\"startdate\">Date:</label>".
	//	"<input type=\"text\" id=\"startdate\" value=\"$row[3]\"> ".
		"<input type=\"text\" name=\"startdate\"  id=\"startdate\"> ".
		"</span>".
		"<span class=\"form-group form-group-sm div-scheduled\" id=\"div_repeatafter\" style=\"".(($row[3] == 1) ?  "display:inline" : "display:none")."\">".
		"<label class=\"control-label\" for=\"repeatafter\">Repeat after:</label> <input id=\"repeatafter\"  name=\"repeatafter\" value=\"".$row[5]."\" />".
		"</span>".
		"<span class=\"form-group form-group-sm div-scheduled\" id=\"div_oneverydate\" style=\"".(($row[3] == 2) ?  "display:inline" : "display:none")."\">".
		"<label class=\"control-label\" for=\"oneverydate\">On every:</label> <input id=\"oneverydate\" name=\"oneverydate\" value=\"$row[6]\"/>".
		"</span>".
		"<span class=\"form-group form-group-sm div-scheduled schedule_format\">".
		"<input type=\"submit\" class=\"btn btn-primary\" name=\"schedule_edit\" value=\"Save\">".
		"</span>".
		"</form>".
		"</div>".
		"</div>";
echo $div;
	}
		
}

?>
