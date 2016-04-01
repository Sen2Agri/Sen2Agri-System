<?php
function add_new_scheduled_jobs_layout_LAI($processorId) {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$sql = "SELECT * FROM sp_get_sites()";
	$result = pg_query ( $db, $sql ) or die ( "Could not execute." );

	$option_site = "";
	while ( $row = pg_fetch_row ( $result ) ) {
		$option ="<option value=\"".$row [0]."\">".$row [1]."</option>";

		$option_site = $option_site . $option;
	}

	if ($processorId == '2') {
		$action = "dashboard.php#tab_l3a";
	} elseif ($processorId == '3') {
		$action = "dashboard.php#tab_l3b";
	} elseif ($processorId == '4') {
		$action = "dashboard.php#tab_l4a";
	} elseif ($processorId == '5') {
		$action = "dashboard.php#tab_l4b";
	}elseif ($processorId == '7'){
		$action = "dashboard.php#tab_l3e_pheno";
	}

	$div =<<<ADDJOB
	<div class="panel_job_container">
	<div class="panel panel-default panel_job">
	<form role="form" name="jobform" id="jobform" method="post" action="$action" style="padding:10px;">
	<span class="form-group form-group-sm span-scheduled" style=" max-width: 1000px;">
	<label class="control-label control-max-width-lai" for="jobname">Job Name: <input type="text" name="jobname" id="jobname" class="schedule_format">
	</label></span>
	<input type="hidden" value="$processorId" name="processorId">
	<span class="form-group form-group-sm span-scheduled">
	<label class="control-label control-max-width-lai" for="sitename">Site:
	<span class="schedule_format">
	<select id="sitename" name="sitename">
	<option value="" selected>Select site</option>$option_site
	</select>
	</span></label>
	</span>
	
	<span class="form-group form-group-sm span-scheduled">
	<label class="control-label control-max-width-lai" for="product_add"> Product:
	<span class="schedule_format">
	<select id="product_add$processorId" name="product_add" required="true">
	<option value="" selected>Select a product</option>
	<option value="L3B">L3B</option>
	<option value="L3C">L3C</option>
	<option value="L3D">L3D</option>
	</select>
	</span></label>
	</span>
	
	<span class="form-group form-group-sm span-scheduled">
	<label class="control-label control-max-width-lai" for="schedule"> Schedule:
	<span class="schedule_format">
	<select id="schedule_add$processorId" class="selectedValue" name="schedule_add"  onchange="selectedScheduleAdd($processorId)">
	<option value="" selected>Select a schedule</option>
	<option value="0">Once</option>
	<option value="1">Cycle</option>
	<option value="2">Repeat</option>
	</select>
	</span></label>
	</span>
		
	<span class="form-group form-group-sm span-scheduled" id="div_startdate$processorId" style="display:none">
	<label class="control-label control-max-width-lai" for="startdate">Date:
	<input type="text" name="startdate" id="add_startdate"   class="schedule_format">
	</label></span>
	<span class="form-group form-group-sm span-scheduled" id="div_repeatafter$processorId" style="display:none">
	<label class="control-label control-max-width-lai" for="repeatafter">Repeat after:
	<input type="number" class="schedule_format" id="repeatafter"  name="repeatafter" value="" min="1" step="1"/>
	</label></span>
	<span class="form-group form-group-sm span-scheduled" id="div_oneverydate$processorId" style="display:none">
	<label class="control-label control-max-width-lai"  for="oneverydate">On every:
	<input type="number" class="schedule_format" id="oneverydate" name="oneverydate" min="1" max="31" step="1" value=""/>
	</label></span>
	<span class="form-group form-group-sm span-scheduled schedule_format">
	<input type="submit" class="btn btn-primary" name="schedule_saveJob" value="Save">
	</span>
	</form>
	</div>
	</div>
ADDJOB;
	//ending heredoc; note:no space after ending of heredoc
	echo $div;
	$_SESSION['proc_id'] = $processorId;
}

function update_scheduled_jobs_layout_LAI($processor_id) {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

	/*
	 * schedule type
	 * once = 0,
	 * cycle = 1,
	 * repeat=2
	 */

	if ($processor_id == '2') {
		$action = "dashboard.php#tab_l3a";
	} elseif ($processor_id == '3') {
		$action = "dashboard.php#tab_l3b";
	} elseif ($processor_id == '4') {
		$action = "dashboard.php#tab_l4a";
	} elseif ($processor_id == '5') {
		$action = "dashboard.php#tab_l4b";
	}elseif ($processor_id == '7'){
		$action = "dashboard.php#tab_l3e_pheno";
	}

	$sql = "SELECT * from sp_get_dashboard_processor_scheduled_task('$processor_id')";
	$result = pg_query ( $db, $sql ) or die ( "Could not execute." );

	$counter = "0";
	while ( $row = pg_fetch_row ( $result ) ) {

		if ($row[3] == 0) {
			$startdate = "display:inline-block";
			$repeatafter = "display:none";
			$oneverydate = "display:none";

			$selected_once ="selected";
			$selected_cycle ="";
			$selected_repeat ="";

		}elseif ($row[3] == 1){
			$startdate = "display:inline-block";
			$repeatafter = "display:inline-block";
			$oneverydate = "display:none";

			$selected_once ="";
			$selected_cycle ="selected";
			$selected_repeat ="";


		}elseif ($row[3] == 2){
			$startdate = "display:inline-block";
			$repeatafter = "display:none";
			$oneverydate = "display:inline-block";

			$selected_once ="";
			$selected_cycle ="";
			$selected_repeat ="selected";

		}
		
		$product = json_decode($row[7],true);
		$prod = $product['general_params']['product_type'];
		$selected_L3B ="";
		$selected_L3C ="";
		$selected_L3D ="";
		if ($prod == 'L3B') {
			$selected_L3B ="selected";
			$selected_L3C ="";
			$selected_L3D ="";
		}elseif ($prod == 'L3C') {
			$selected_L3B ="";
			$selected_L3C ="selected";
			$selected_L3D ="";
		}elseif ($prod == 'L3D') {
			$selected_L3B ="";
			$selected_L3C ="";
			$selected_L3D ="selected";
		
		}

		// Set up the DOM elements
		// heredoc
		$div2 = <<<JOB
		<div class="panel_job_container">
		<div class="panel panel-default panel_job">
		<form class="form-inline sched_form" role="form" name="jobform_edit" id="jobform_edit" method="post" action="$action" style="padding:10px;" data-counter="$counter">
		<span class="form-group form-group-sm span-scheduled" style=" max-width: 1000px;">
		<label class="control-label control-max-width-lai">Job Name: <span class="schedule_format">$row[1]</span>
		</label>
		<input type="hidden" value="$row[0]" name="scheduledID">
		<input type="hidden" value="$processor_id" name="processor">
		<span class="form-group form-group-sm span-scheduled">
		<label class="control-label control-max-width-lai" >Site: <span class="schedule_format">$row[2]</span></label>
		</span>

		<span class="form-group form-group-sm span-scheduled">
		<label class="control-label control-max-width-lai"> Product:
		<span class="schedule_format">
		<select id="product$row[0]" name="product" onchange="activateButton($row[0])">
		<option value="L3B" $selected_L3B> L3B </option>
		<option value="L3C" $selected_L3C> L3C </option>
		<option value="L3D" $selected_L3D> L3D </option>
		</select>
		</span></label>
		</span>
	
		<span class="form-group form-group-sm span-scheduled">
		<label class="control-label control-max-width-lai" > Schedule:
		<span class="schedule_format">
		<select id="schedule$row[0]" name="schedule" onchange="selectedSchedule($row[0])">
		<option value="0" $selected_once >Once</option>
		<option value="1" $selected_cycle >Cycle</option>
		<option value="2" $selected_repeat >Repeat</option>
		</select>
		</span></label>
		</span>
		
		<span class="form-group form-group-sm span-scheduled" id="div_startdate$row[0]" style="$startdate">
		<label class="control-label control-max-width-lai" >Date:
		<input type="text" class="startdate schedule_format" name="startdate" id="startdate$row[0]" value="$row[4]" onChange="activateButton($row[0])" >
		</label></span>
		<span class="form-group form-group-sm span-scheduled" id="div_repeatafter$row[0]" style="$repeatafter">
		<label class="control-label control-max-width-lai">Repeat after :
		<input type="number" class="schedule_format" name="repeatafter" id="repeatafter$row[0]" value="$row[5]" onChange="checkMin($row[0])" min="0"/>
		</label></span>
		<span class="form-group form-group-sm span-scheduled" id="div_oneverydate$row[0]" style="$oneverydate">
		<label class="control-label control-max-width-lai">On every:
		<input type="number" class="schedule_format" name="oneverydate" id="oneverydate$row[0]" value="$row[6]" onChange="setMin($row[0])" min="0" max="31" step="1"/>
		</label></span>
		<span class="form-group form-group-sm span-scheduled schedule_format">
		<input type="submit" class="btn btn-primary" name="schedule_submit" id="schedule_submit$row[0]" value="Save" disabled>
		</span>
		</form>
		</div>
		</div>
JOB;
		// end of heredoc

		echo $div2;
		$counter++;
	}
}


function add_new_scheduled_jobs_layout($processorId) {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$sql = "SELECT * FROM sp_get_sites()";
	$result = pg_query ( $db, $sql ) or die ( "Could not execute." );
	
	$option_site = "";
	while ( $row = pg_fetch_row ( $result ) ) {
		$option ="<option value=\"".$row [0]."\">".$row [1]."</option>";
				
		$option_site = $option_site . $option;
	}
	
	if ($processorId == '2') {
		$action = "dashboard.php#tab_l3a";
	} elseif ($processorId == '3') {
		$action = "dashboard.php#tab_l3b";
	} elseif ($processorId == '4') {
		$action = "dashboard.php#tab_l4a";
	} elseif ($processorId == '5') {
		$action = "dashboard.php#tab_l4b";
	}elseif ($processorId == '7'){
		$action = "dashboard.php#tab_l3e_pheno";
	}

$div =<<<ADDJOB
	<div class="panel_job_container">
	<div class="panel panel-default panel_job">
	<form role="form" name="jobform" id="jobform" method="post" action="$action" style="padding:10px;">
	<span class="form-group form-group-sm span-scheduled" style=" max-width: 1000px;">
	<label class="control-label control-max-width" for="jobname">Job Name: <input type="text" name="jobname" id="jobname" class="schedule_format">
	</label></span>
	<input type="hidden" value="$processorId" name="processorId">
	<span class="form-group form-group-sm span-scheduled">
	<label class="control-label control-max-width" for="sitename">Site: 
	<span class="schedule_format">
	<select id="sitename" name="sitename">
	<option value="" selected>Select site</option>$option_site
	</select>
	</span></label>
	</span>
	<span class="form-group form-group-sm span-scheduled">
	<label class="control-label control-max-width" for="schedule"> Schedule: 
	<span class="schedule_format">
	<select id="schedule_add$processorId" class="selectedValue" name="schedule_add"  onchange="selectedScheduleAdd($processorId)">
	<option value="" selected>Select a schedule</option>
	<option value="0">Once</option>
	<option value="1">Cycle</option>
	<option value="2">Repeat</option>
	</select>
	</span></label>
	</span>
	<span class="form-group form-group-sm span-scheduled" id="div_startdate$processorId" style="display:none">
	<label class="control-label" for="startdate" style="display:inline-block;width:150px;">Date: 
	<input type="text" name="startdate" id="add_startdate"   class="schedule_format">
	</label></span>
	<span class="form-group form-group-sm span-scheduled" id="div_repeatafter$processorId" style="display:none">
	<label class="control-label" for="repeatafter" style="display:inline-block;width:150px;">Repeat after: 
	<input type="number" class="schedule_format" id="repeatafter"  name="repeatafter" value="" min="1" step="1"/>
	</label></span>
	<span class="form-group form-group-sm span-scheduled" id="div_oneverydate$processorId" style="display:none">
	<label class="control-label" for="oneverydate" style="display:inline-block;width:150px;">On every: 
	<input type="number" class="schedule_format" id="oneverydate" name="oneverydate" min="1" max="31" step="1" value=""/>
	</label></span>
	<span class="form-group form-group-sm span-scheduled schedule_format">
	<input type="submit" class="btn btn-primary" name="schedule_saveJob" value="Save">
	</span>
	</form>
	</div>
	</div>
ADDJOB;
//ending heredoc; note:no space after ending of heredoc	
	echo $div;
	$_SESSION['proc_id'] = $processorId;
}

function update_scheduled_jobs_layout($processor_id) {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		
	/*
	 * schedule type
	 * once = 0,
	 * cycle = 1,
	 * repeat=2
	 */
	
	if ($processor_id == '2') {
		$action = "dashboard.php#tab_l3a";
	} elseif ($processor_id == '3') {
		$action = "dashboard.php#tab_l3b";
	} elseif ($processor_id == '4') {
		$action = "dashboard.php#tab_l4a";
	} elseif ($processor_id == '5') {
		$action = "dashboard.php#tab_l4b";
	}elseif ($processor_id == '7'){
		$action = "dashboard.php#tab_l3e_pheno";
	}

	$sql = "SELECT * from sp_get_dashboard_processor_scheduled_task('$processor_id')";
	$result = pg_query ( $db, $sql ) or die ( "Could not execute." );
	
	$counter = "0";
	while ( $row = pg_fetch_row ( $result ) ) {
		
			if ($row[3] == 0) {
				$startdate = "display:inline-block";
				$repeatafter = "display:none";
				$oneverydate = "display:none";
				
				$selected_once ="selected";
				$selected_cycle ="";
				$selected_repeat ="";

			}elseif ($row[3] == 1){
				$startdate = "display:inline-block";
				$repeatafter = "display:inline-block";
				$oneverydate = "display:none";
				
				$selected_once ="";
				$selected_cycle ="selected";
				$selected_repeat ="";
				

			}elseif ($row[3] == 2){
				$startdate = "display:inline-block";
				$repeatafter = "display:none";
				$oneverydate = "display:inline-block";
				
				$selected_once ="";
				$selected_cycle ="";
				$selected_repeat ="selected";
				
			}
			
		
		// Set up the DOM elements
		// heredoc 
		$div2 = <<<JOB
		<div class="panel_job_container">
		<div class="panel panel-default panel_job">
		<form class="form-inline sched_form" role="form" name="jobform_edit" id="jobform_edit" method="post" action="$action" style="padding:10px;" data-counter="$counter">
		<span class="form-group form-group-sm span-scheduled" style=" max-width: 1000px;">
		<label class="control-label control-max-width">Job Name: <span class="schedule_format">$row[1]</span>
		</label>
		<input type="hidden" value="$row[0]" name="scheduledID">
		<span class="form-group form-group-sm span-scheduled">
		<label class="control-label control-max-width" >Site: <span class="schedule_format">$row[2]</span></label>
		</span>
		<span class="form-group form-group-sm span-scheduled">
		<label class="control-label control-max-width" > Schedule:
		<span class="schedule_format">
		<select id="schedule$row[0]" name="schedule" onchange="selectedSchedule($row[0])">
		<option value="0" $selected_once >Once</option>
		<option value="1" $selected_cycle >Cycle</option>
		<option value="2" $selected_repeat >Repeat</option>
		</select>
		</span></label>
		</span>
		<span class="form-group form-group-sm span-scheduled" id="div_startdate$row[0]" style="$startdate">
		<label class="control-label " style="display:inline-block;width:150px;">Date:
		<input type="text" class="startdate schedule_format" name="startdate" id="startdate$row[0]" value="$row[4]" onChange="activateButton($row[0])" >
		</label></span>
		<span class="form-group form-group-sm span-scheduled" id="div_repeatafter$row[0]" style="$repeatafter">
		<label class="control-label" style="display:inline-block;width:150px;">Repeat after :
		<input type="number" class="schedule_format" name="repeatafter" id="repeatafter$row[0]" value="$row[5]" onChange="checkMin($row[0])" min="0"/>
		</label></span>
		<span class="form-group form-group-sm span-scheduled" id="div_oneverydate$row[0]" style="$oneverydate">
		<label class="control-label" style="display:inline-block;width:150px;">On every: 
		<input type="number" class="schedule_format" name="oneverydate" id="oneverydate$row[0]" value="$row[6]" onChange="setMin($row[0])" min="0" max="31" step="1"/>
		</label></span>
		<span class="form-group form-group-sm span-scheduled schedule_format">
		<input type="submit" class="btn btn-primary" name="schedule_submit" id="schedule_submit$row[0]" value="Save" disabled>
		</span>
		</form>
		</div>
		</div>
JOB;
// end of heredoc

		echo $div2;
		$counter++;
	}
}

?>
