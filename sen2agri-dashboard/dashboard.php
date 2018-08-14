<?php include "master.php"; ?>
<?php include "dashboardCreatJobs.php"; ?>
<?php

if (isset($_REQUEST['schedule_add']) && isset($_REQUEST['processorId'])) {
	if ($_REQUEST['schedule_add'] == 'Add New Job') {
		$active_proc = $_REQUEST ['processorId'] + 0;
	}
}

// Submited add new job; insert job in database with id $schedule_id
if (isset ( $_REQUEST ['schedule_saveJob'] ) && $_REQUEST ['schedule_saveJob'] == 'Save') {
    $db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );

	$job_name      = $_REQUEST ['jobname'];
	$processorId   = $_REQUEST ['processorId'];
    $site_id       = $_REQUEST ['sitename'];
    $season_id     = $_REQUEST ['seasonname'];
	$schedule_type = $_REQUEST ['schedule_add'];

	$startdate="";
	if ($schedule_type == '0') {
		$repeatafter = "0";
		$oneverydate = "0";
		$startdate = $_REQUEST ['startdate'];
	} elseif ($schedule_type == '1') {
		$repeatafter = $_REQUEST ['repeatafter'];
		$oneverydate = "0";
		$startdate = $_REQUEST ['startdate'];
	} elseif ($schedule_type == '2') {
		$repeatafter = "0";
		$oneverydate = $_REQUEST ['oneverydate'];
		$startdate = $_REQUEST ['startdate'];
	}

	$pg_date = date ( 'Y-m-d H:i:s', strtotime ( $startdate ) );

	$retry_seconds = 60;
	$priority = 1;
	$processor_params = null;

	if($processorId == '3'){
	$processor_params = json_encode ( array (
			"general_params" => array (
					"product_type" =>  $_REQUEST['product_add']))
			);
	}
	//save new job in database
	$res = pg_query_params ( $db, "SELECT sp_insert_scheduled_task($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11)", array (
			$job_name,
			$processorId,
            $site_id,
            $season_id,
			$schedule_type,
			$repeatafter,
			$oneverydate,
			$pg_date,
			$retry_seconds,
			$priority,
			$processor_params
	) )or die ( "An error occurred." );

	ob_clean();
	echo "SUCCESS: Added new job for processor " . $processorId;
	exit;
}

// Submited edit job; update job with id $schedule_id in database
if (isset ( $_REQUEST ['schedule_submit'] ) && $_REQUEST ['schedule_submit'] == 'Save') {
    $db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );

	$schedule_id = $_REQUEST ['scheduledID'];
	$schedule_type = $_REQUEST ['schedule'];
	$startdate = $_REQUEST ['startdate'];
	$processorId = $_REQUEST ['processorId'];

	$processor_params = null;
	//if L3B and sen2agri DB
	if(ConfigParams::isSen2Agri() && $processorId == '3'){
		$processor_params = json_encode ( array (
				"general_params" => array (
						"product_type" => $_REQUEST['product']))
				);
	}

	$pg_date = date ( 'Y-m-d H:i:s', strtotime ( $startdate ) );

	if ($schedule_type == '0') {
		$repeatafter = "0";
		$oneverydate = "0";
		$startdate = $_REQUEST ['startdate'];
	} elseif ($schedule_type == '1') {
		$repeatafter = $_REQUEST ['repeatafter'];
		$oneverydate = "0";
		$startdate = $_REQUEST ['startdate'];
	} elseif ($schedule_type == '2') {
		$repeatafter = "0";
		$oneverydate = $_REQUEST ['oneverydate'];
		$startdate = $_REQUEST ['startdate'];
	}

	//update scheduled task
	$res = pg_query_params ( $db, "SELECT sp_dashboard_update_scheduled_task($1,$2,$3,$4,$5,$6)", array (
			$schedule_id,
			$schedule_type,
			$repeatafter,
			$oneverydate,
			$pg_date,
			$processor_params
	) )or die ( "An error occurred." );

	$active_proc = $processorId + 0;
}

// Submited remove job; remove job with id $schedule_id in database
if (isset ( $_REQUEST ['schedule_submit_delete'] ) && $_REQUEST ['schedule_submit_delete'] == 'Delete') {	
    $db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );

	$schedule_id = $_REQUEST ['scheduledID'];
	
	//remove task from scheduled task
	$res = pg_query_params ( $db, "SELECT sp_dashboard_remove_scheduled_task($1)", array (
			$schedule_id ) )or die ( "An error occurred." );

	$processorId = $_REQUEST ['processorId'];
	$active_proc = $processorId + 0;

}


$all_processors = array();

$db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
$sql = "SELECT id, short_name, name as label FROM processor WHERE short_name NOT LIKE 'l2%'";
$rows = pg_query($db, $sql) or die ("An error occurred.");
$all_processors =  pg_fetch_all($rows);

?>

<div id="main">
	<div id="main2">
		<div id="main3">
			<div id="content" style="width: 100%;">
				<div id="tab_control" class="tabControl">
					<!-- L2A Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<!-- <div id="tab_l2a">
						<a href=""<?= $active_proc ==  1 ? " class='active'" : "" ?>>L2A Processor</a>
						<div class="panel">
							<div class="panel_resources_and_output_container dash_panel" id="pnl_l2a_resources_and_output_container">
								<div class="panel_resources_container" id="pnl_l2a_resources_container">
									<div class="panel panel-default panel_resources" id="pnl_l2a_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container" id="pnl_l2a_output_container">
									<div class="panel panel-default panel_output" id="pnl_l2a_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container dash_panel" id="pnl_l2a_configuration_container">
								<div class="panel panel-default panel_configuration" id="pnl_l2a_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>

						</div>
					</div> -->
					<!-- L3A Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<?php 
					$processorList = array();
					if(sizeof($all_processors)>0){
					    
					    foreach ($all_processors as $processor){
					        if(!isset($active_proc)){
					            if(isset($_SESSION['active_proc'])){
					                $active_proc = $_SESSION['active_proc'];
    					            unset($_SESSION['active_proc']);
					            }else{
					                $active_proc = $processor['id'];
					            }
					        }
					        $proc_id = $processor['id'];
					        $proc_name = $processor['short_name'];
					        $processorList[] = $proc_name;
					        $nr_proc = sizeof($all_processors);
					      //  $minWidth = 100/$nr_proc;
					?>
					<div id="tab_<?=$proc_name ?>"  class="proc_tab " style="width:<?=1018/$nr_proc ?>px;">
						<a href=""<?=  $active_proc==  $processor['id'] ? " class='active'" : "" ?> style="width:85%;white-space: nowrap; overflow: hidden; text-overflow: ellipsis;" title="<?=  $processor['label']?>"><?=  $processor['label']?> </a>
						<div >
							<div class="panel_resources_and_output_container dash_panel" id="pnl_<?=$proc_name ?>_resources_and_output_container">
								<div class="panel_resources_container" id="pnl_<?=$proc_name ?>_resources_container">
									<div class="panel panel-default panel_resources" id="pnl_<?=$proc_name ?>_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container" id="pnl_<?=$proc_name ?>_output_container">
									<div class="panel panel-default panel_output" id="pnl_<?=$proc_name ?>_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container dash_panel" id="pnl_<?=$proc_name ?>_configuration_container">
								<div class="panel panel-default panel_configuration" id="pnl_<?=$proc_name ?>_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container dash_panel" id="pnl_<?=$proc_name ?>_scheduled_container">
								<div class="panel panel-default panel_scheduled" id="pnl_<?=$proc_name ?>_scheduled">
									<div class="schedule-table add-button">
										<form id="form_add_sched_<?=$proc_name ?>" method="post" class="form">
											<input type="hidden" name="processorId" value="<?=$proc_id ?>">
											<input class="button add-edit-btn" name="schedule_add" type="submit" value="Add New Job">
										</form>
									</div>
									
									<div class="schedule-table">
										<?php
										get_scheduled_jobs_header($proc_id);
										if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
										    if ($_REQUEST ['schedule_add'] == 'Add New Job' && $_REQUEST ['processorId'] == $proc_id) {
										        add_new_scheduled_jobs_layout($proc_id);
											}
										}
										update_scheduled_jobs_layout($proc_id);
										?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
						</div>
						</div>
										<?php 
					    }
					}?>
							
				</div>
			</div>
			<div class="clearing">&nbsp;</div>
		</div>
	</div>
</div>
<script>var processorList = [];
processorList = <?php echo json_encode( $processorList)?>;
</script>

<script src="scripts/helpers.js"></script>
<script src="scripts/processing_functions.js"></script>
<script src="scripts/processing.js"></script>

<link rel="stylesheet" href="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/themes/smoothness/jquery-ui.css">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
<script src="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/jquery-ui.min.js"></script>
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>

<script>
	function selectedSchedule(event, param) {
		var target = event.target;
		var row = target.parentElement.parentElement;
		var div_startdate = row.getElementsByClassName("div_startdate")[0];
		var div_repeatnever = row.getElementsByClassName("div_repeatnever")[0];
		var div_repeatafter = row.getElementsByClassName("div_repeatafter")[0];
		var div_oneverydate = row.getElementsByClassName("div_oneverydate")[0];
		var div_startdatefoo = row.getElementsByClassName("div_startdatefoo")[0];

		switch (target.value) {
			case "0": // Once
				div_repeatnever.classList.remove("hidden");
				div_repeatafter.classList.add("hidden");
				div_oneverydate.classList.add("hidden");
				break;
			case "1":// Cycle
				div_repeatnever.classList.add("hidden");
				div_repeatafter.classList.remove("hidden");
				div_oneverydate.classList.add("hidden");
				break;
			case "2"://Repeat
				div_repeatnever.classList.add("hidden");
				div_repeatafter.classList.add("hidden");
				div_oneverydate.classList.remove("hidden");
				break;
			default: //once by default
				div_repeatnever.classList.remove("hidden");
				div_repeatafter.classList.add("hidden");
				div_oneverydate.classList.add("hidden");
				break;
		}
        activateButton(param);
	}
	function selectedScheduleAdd(event, param) {
		var target = event.target;
		var row = target.parentElement.parentElement;
		var div_startdate = row.getElementsByClassName("div_startdate")[0];
		var div_repeatnever = row.getElementsByClassName("div_repeatnever")[0];
		var div_repeatafter = row.getElementsByClassName("div_repeatafter")[0];
		var div_oneverydate = row.getElementsByClassName("div_oneverydate")[0];
		var div_startdatefoo = row.getElementsByClassName("div_startdatefoo")[0];
		var div_repeatfoo = row.getElementsByClassName("div_repeatfoo")[0];

		switch (target.value) {
			case "0":
				div_startdate.classList.remove("hidden");
				div_repeatnever.classList.remove("hidden");

				div_repeatafter.classList.add("hidden");
				div_oneverydate.classList.add("hidden");

				div_startdatefoo.classList.add("hidden");
				div_repeatfoo.classList.add("hidden");
				break;
			case "1":
				div_startdate.classList.remove("hidden");
				div_repeatafter.classList.remove("hidden");

				div_repeatnever.classList.add("hidden");
				div_oneverydate.classList.add("hidden");

				div_startdatefoo.classList.add("hidden");
				div_repeatfoo.classList.add("hidden");
				break;
			case "2":
				div_startdate.classList.remove("hidden");
				div_oneverydate.classList.remove("hidden");

				div_repeatnever.classList.add("hidden");
				div_repeatafter.classList.add("hidden");

				div_startdatefoo.classList.add("hidden");
				div_repeatfoo.classList.add("hidden");
				break;
			default:
				div_startdatefoo.classList.remove("hidden");
				div_repeatfoo.classList.remove("hidden");

				div_repeatnever.classList.add("hidden");
				div_repeatafter.classList.add("hidden");
				div_oneverydate.classList.add("hidden");
				div_startdate.classList.add("hidden");
				break;
		}
	}
	function selectedSiteAdd(param) {
		var site_id = $("#sitename"+param).val();
		$("#seasonname"+param).val("");
		$("#seasonname"+param).find("option").each(function( index ) {
			if ($(this).data("siteid") == site_id || $(this).val() == "") {
				$(this).removeClass("hidden");
			} else {
				$(this).addClass("hidden");
			}
		});
	}
	function activateButton(x){
		document.getElementById("schedule_submit"+x).disabled = false;
	}
	function checkMin(y){
		var val = document.getElementById("repeatafter"+y).value;
		if( val < 1){
			alert("Invalid number");
			document.getElementById("schedule_submit"+y).disabled = true;
			}
		else {
			document.getElementById("schedule_submit"+y).disabled = false;
		}
	}
	function setMin(x){
		var val = document.getElementById("oneverydate"+x).value;
		if(val<1 || val>31){
			alert("Number should be between[1,31]");
			 document.getElementById("schedule_submit"+x).disabled = true;
		}else {
			document.getElementById("schedule_submit"+x).disabled = false;
		}
	}
	$(document).ready(function() {
		// START tab control
		$('.tabControl>div>a.active').parent().find(">div").css("z-index", "999");
		$('.tabControl>div>a').click(function(e) {
			e.preventDefault();
			// set active tab header and deactivate all other headers
			$('.tabControl>div>a').removeClass("active");
			$(this).addClass("active");
			// display current tab content and hide all other tabs
			$('.tabControl>div>div').css("z-index", "-1");
			$(this).parent().find(">div").css("z-index", "999");

			return false;
		});
		// END tab control

		$( ".startdate" ).datepicker({
			dateFormat: "yy-mm-dd"
		});
		<?php if (isset($_SESSION['proc_id'])) { ?>
			$("#jobform").validate({
				rules: {
					jobname:      { required: true, pattern: "[a-zA-Z]{1}[a-zA-Z0-9_]*" },
					sitename:     { required: true },
					seasonname:   { required: true },
					product_add:  { required: true },
					schedule_add: { required: true },
				},
				messages: {
					jobname: { pattern : "First character must be a letter. Only letters, numbers and underscore are allowed." }
				},
				highlight: function(element, errorClass) {
					$(element).parent().addClass("has-error");
				},
				unhighlight: function(element, errorClass) {
					$(element).parent().removeClass("has-error");
				},
				errorPlacement: function(error, element) {
					error.appendTo(element.parent());
				},
				submitHandler :function(form) {
					$.ajax({
						url:     $(form).attr('action'),
						type:    $(form).attr('method'),
						data:    $(form).serialize(),
						success: function(response) {
							if (response.startsWith("SUCCESS")) {
								alert(response);
							} else {
								alert("An error occured while adding the new job.");
							}
							window.location.href = "dashboard.php";
						}
					});
				},
				success : function(label) {
					label.remove();
				}
			});
		<?php 
		//keep the current tab in session variable active_proc
		$_SESSION['active_proc'] = $_SESSION['proc_id'];
		unset($_SESSION['proc_id']); } ?>
	});
</script>

<?php include "ms_foot.php"; ?>
