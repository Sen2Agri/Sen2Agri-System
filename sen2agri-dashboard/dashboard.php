<?php include "master.php"; ?>
<?php include "dashboardCreatJobs.php"; ?>
<?php

$active_proc = 2;
if (isset($_REQUEST['schedule_add']) && isset($_REQUEST['processorId'])) {
	if ($_REQUEST['schedule_add'] == 'Add New Job') {
		$active_proc = $_REQUEST ['processorId'] + 0;
	}
}

// Submited add new job; insert job in database with id $schedule_id
if (isset ( $_REQUEST ['schedule_saveJob'] ) && $_REQUEST ['schedule_saveJob'] == 'Save') {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

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
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

	$schedule_id = $_REQUEST ['scheduledID'];
	$schedule_type = $_REQUEST ['schedule'];
	$startdate = $_REQUEST ['startdate'];
	$processorId = $_REQUEST ['processorId'];

	$processor_params = null;
	if($processorId == '3'){
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
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

	$schedule_id = $_REQUEST ['scheduledID'];
	
	//remove task from scheduled task
	$res = pg_query_params ( $db, "SELECT sp_dashboard_remove_scheduled_task($1)", array (
			$schedule_id ) )or die ( "An error occurred." );

	$processorId = $_REQUEST ['processorId'];
	$active_proc = $processorId + 0;

}
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
					<div id="tab_l3a">
						<a href=""<?= $active_proc ==  2 ? " class='active'" : "" ?>>L3A &mdash; Cloud-free Composite</a>
						<div>
							<div class="panel_resources_and_output_container dash_panel" id="pnl_l3a_resources_and_output_container">
								<div class="panel_resources_container" id="pnl_l3a_resources_container">
									<div class="panel panel-default panel_resources" id="pnl_l3a_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container" id="pnl_l3a_output_container">
									<div class="panel panel-default panel_output" id="pnl_l3a_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container dash_panel" id="pnl_l3a_configuration_container">
								<div class="panel panel-default panel_configuration" id="pnl_l3a_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container dash_panel" id="pnl_l3a_scheduled_container">
								<div class="panel panel-default panel_scheduled" id="pnl_l3a_scheduled">
									<div class="schedule-table add-button">
										<form id="form_add_sched_l3a" method="post" class="form">
											<input type="hidden" name="processorId" value="2">
											<input class="button add-edit-btn" name="schedule_add" type="submit" value="Add New Job">
										</form>
									</div>
									<!-- l3a processor_id = 2 -->
									<div class="schedule-table">
										<?php
										get_scheduled_jobs_header(2);
										if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
											if ($_REQUEST ['schedule_add'] == 'Add New Job' && $_REQUEST ['processorId'] == '2') {
												add_new_scheduled_jobs_layout(2);
											}
										}
										update_scheduled_jobs_layout(2);
										?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
						</div>
					</div>
					<!-- L3B Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l3b">
						<a href=""<?= $active_proc ==  3 ? " class='active'" : "" ?>>L3B &mdash; LAI/NDVI</a>
						<div>
							<div class="panel_resources_and_output_container dash_panel" id="pnl_l3b_resources_and_output_container">
								<div class="panel_resources_container"
									id="pnl_l3b_resources_container">
									<div class="panel panel-default panel_resources" id="pnl_l3b_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container" id="pnl_l3b_output_container">
									<div class="panel panel-default panel_output" id="pnl_l3b_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container dash_panel" id="pnl_l3b_configuration_container">
								<div class="panel panel-default panel_configuration" id="pnl_l3b_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container dash_panel" id="pnl_l3b_scheduled_container">
								<div class="panel panel-default panel_scheduled" id="pnl_l3b_scheduled">
									<div class="schedule-table add-button">
										<form id="form_add_sched_l3b" method="post">
											<input type="hidden" name="processorId" value="3">
											<input class="button add-edit-btn" name="schedule_add" type="submit" value="Add New Job">
										</form>
									</div>
									<!-- l3b lai processor_id = 3 -->
									<div class="schedule-table">
										<?php
										get_scheduled_jobs_header(3);
										if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
											if ($_REQUEST ['schedule_add'] == 'Add New Job' && $_REQUEST ['processorId'] == '3') {
												add_new_scheduled_jobs_layout(3);
											}
										}
										update_scheduled_jobs_layout(3);
										?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
						</div>
					</div>
					<!-- L3b pheno NDVI Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l3e_pheno">
						<a href=""<?= $active_proc ==  4 ? " class='active'" : "" ?>>L3E &mdash; Phenology Indices</a>
						<div>
							<div class="panel_resources_and_output_container dash_panel" id="pnl_l3e_pheno_resources_and_output_container">
								<div class="panel_resources_container" id="pnl_l3e_pheno_resources_container">
									<div class="panel panel-default panel_resources" id="pnl_l3e_pheno_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container" id="pnl_l3e_pheno_output_container">
									<div class="panel panel-default panel_output" id="pnl_l3e_pheno_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container dash_panel" id="pnl_l3e_pheno_configuration_container">
								<div class="panel panel-default panel_configuration" id="pnl_l3e_pheno_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container dash_panel" id="pnl_l3e_pheno_scheduled_container">
								<div class="panel panel-default panel_scheduled" id="pnl_l3e_pheno_scheduled">
									<div class="schedule-table add-button">
										<form id="form_add_sched_l3e_pheno" method="post">
											<input type="hidden" name="processorId" value="4">
											<input class="button add-edit-btn" name="schedule_add" type="submit" value="Add New Job">
										</form>
									</div>
									<!-- l3e pheno NDVI processor_id = 4 -->
									<div class="schedule-table">
										<?php
										get_scheduled_jobs_header(4);
										if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
											if ($_REQUEST ['schedule_add'] == 'Add New Job' && $_REQUEST ['processorId'] == '4') {
												add_new_scheduled_jobs_layout(4);
											}
										}
										update_scheduled_jobs_layout(4);
										?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
						</div>
					</div>
					<!-- L4A Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l4a">
						<a href=""<?= $active_proc ==  5 ? " class='active'" : "" ?>>L4A &mdash; Cropland Mask</a>
						<div>
							<div class="panel_resources_and_output_container dash_panel" id="pnl_l4a_resources_and_output_container">
								<div class="panel_resources_container" id="pnl_l4a_resources_container">
									<div class="panel panel-default panel_resources" id="pnl_l4a_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container" id="pnl_l4a_output_container">
									<div class="panel panel-default panel_output" id="pnl_l4a_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container dash_panel" id="pnl_l4a_configuration_container">
								<div class="panel panel-default panel_configuration" id="pnl_l4a_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container dash_panel" id="pnl_l4a_scheduled_container">
								<div class="panel panel-default panel_scheduled" id="pnl_l4a_scheduled">
									<div class="schedule-table add-button">
										<form id="form_add_sched_l4a" method="post">
											<input type="hidden" name="processorId" value="5">
											<input class="button add-edit-btn" name="schedule_add" type="submit" value="Add New Job">
										</form>
									</div>
									<!-- l4a processor_id = 5 -->
									<div class="schedule-table">
										<?php
										get_scheduled_jobs_header(5);
										if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
											if ($_REQUEST ['schedule_add'] == 'Add New Job' && $_REQUEST ['processorId'] == '5') {
												add_new_scheduled_jobs_layout(5);
											}
										}
										update_scheduled_jobs_layout(5);
										?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
						</div>
					</div>
					<!-- L4B Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l4b">
						<a href=""<?= $active_proc ==  6 ? " class='active'" : "" ?>>L4B &mdash; Crop Type Map</a>
						<div>
							<div class="panel_resources_and_output_container dash_panel" id="pnl_l4b_resources_and_output_container">
								<div class="panel_resources_container" id="pnl_l4b_resources_container">
									<div class="panel panel-default panel_resources" id="pnl_l4b_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container" id="pnl_l4b_output_container">
									<div class="panel panel-default panel_output" id="pnl_l4b_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_scheduled_container" id="pnl_l4b_scheduled_container">
									<div class="panel panel-default panel_output" id="pnl_l4b_scheduled">
										<div class="panel-heading">Scheduled Tasks</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container dash_panel" id="pnl_l4b_configuration_container">
								<div class="panel panel-default panel_configuration" id="pnl_l4b_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container dash_panel" id="pnl_l4b_scheduled_container">
								<div class="panel panel-default panel_scheduled" id="pnl_l4b_scheduled">
									<div class="schedule-table add-button">
										<form id="form_add_sched_l4b" method="post">
											<input type="hidden" name="processorId" value="6">
											<input class="button add-edit-btn" name="schedule_add" type="submit" value="Add New Job">
										</form>
									</div>
									<!-- l4b processor_id = 6 -->
									<div class="schedule-table">
										<?php
										get_scheduled_jobs_header(6);
										if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
											if ($_REQUEST ['schedule_add'] == 'Add New Job' && $_REQUEST ['processorId'] == '6') {
												add_new_scheduled_jobs_layout(6);
											}
										}
										update_scheduled_jobs_layout(6);
										?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
						</div>
					</div>
				</div>
			</div>
			<div class="clearing">&nbsp;</div>
		</div>
	</div>
</div>

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
			case "0":
				div_repeatnever.classList.remove("hidden");
				div_repeatafter.classList.add("hidden");
				div_oneverydate.classList.add("hidden");
				break;
			case "1":
				div_repeatnever.classList.add("hidden");
				div_repeatafter.classList.remove("hidden");
				div_oneverydate.classList.add("hidden");
				break;
			case "2":
				div_repeatnever.classList.add("hidden");
				div_repeatafter.classList.add("hidden");
				div_oneverydate.classList.remove("hidden");
				break;
			default:
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
		<?php unset($_SESSION['proc_id']); } ?>
	});
</script>

<?php include "ms_foot.php"; ?>
