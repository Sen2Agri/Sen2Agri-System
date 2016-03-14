<?php include "master.php"; ?>
<?php

// Submited add new job; insert job in database with id $schedule_id
if (isset ( $_REQUEST ['schedule_saveJob'] ) && $_REQUEST ['schedule_saveJob'] == 'Save') {
	$db = pg_connect ( 'host=' . ConfigParams::$SERVER_NAME . ' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
		
	$job_name = $_REQUEST ['jobname'];
	$processorId = $_REQUEST ['processorId'];
	$site_id = $_REQUEST ['sitename'];
	$schedule_type = $_REQUEST ['schedule_add'];
	
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
	
	//save new job in database
	$sql_insert = "SELECT* sp_insert_scheduled_task($1,$2,$3,$4,$5,$6,$7,$8,$9,$10)";
	$res = pg_prepare ( $db, "my_query", $sql_insert );
	
	$res = pg_execute ( $db, "my_query", array (
			$job_name,
			$processorId,
			$site_id,
			$schedule_type,
			$repeatafter,
			$oneverydate,
			$pg_date,
			$retry_seconds,
			$priority,
			$processor_params
	) ) or die ("An error occurred.");

}

// Submited edit job; update job with id $schedule_id in database
if (isset ( $_REQUEST ['schedule_submit'] ) && $_REQUEST ['schedule_submit'] == 'Save') {
	$db = pg_connect ( 'host=' . ConfigParams::$SERVER_NAME . ' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
		
	$schedule_id = $_REQUEST ['scheduledID'];
	$schedule_type = $_REQUEST ['schedule'];
	$startdate = $_REQUEST ['startdate'];

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
	$sql_update = "SELECT * from sp_dashboard_update_scheduled_task($1,$2,$3,$4,$5)";
	$res = pg_prepare ( $db, "my_query2", $sql_update );
	
	$res = pg_execute ( $db, "my_query2", array (
			$schedule_id,
			$schedule_type,
			$repeatafter,
			$oneverydate,
			$pg_date
	) ) or die ("An error occurred.");

}
?>
<?php include 'dashboardCreatJobs.php';?>


<div id="main">
	<div id="main2">
		<div id="main3">
			<div id="content" style="width: 100%;">
				<div id="tab_control" class="tabControl">
					<!-- L2A Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l2a" style="z-index: 1;">
						<a href="#tab_l2a">L2A Processor</a>
						<div class="panel">
							<div class="panel_resources_and_output_container"
								id="pnl_l2a_resources_and_output_container">
								<div class="panel_resources_container"
									id="pnl_l2a_resources_container">
									<div class="panel panel-default panel_resources"
										id="pnl_l2a_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container"
									id="pnl_l2a_output_container">
									<div class="panel panel-default panel_output"
										id="pnl_l2a_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container"
								id="pnl_l2a_configuration_container">
								<div class="panel panel-default panel_configuration"
									id="pnl_l2a_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
			
						</div>
					</div>
					
					<!-- L3A Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l3a">
						<a href="#tab_l3a">L3A Processor</a>
						<div>
							<div class="panel_resources_and_output_container"
								id="pnl_l3a_resources_and_output_container">
								<div class="panel_resources_container"
									id="pnl_l3a_resources_container">
									<div class="panel panel-default panel_resources"
										id="pnl_l3a_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container"
									id="pnl_l3a_output_container">
									<div class="panel panel-default panel_output"
										id="pnl_l3a_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container"
								id="pnl_l3a_configuration_container">
								<div class="panel panel-default panel_configuration"
									id="pnl_l3a_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>

							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container"
								id="pnl_l3a_scheduled_container">
								<div class="panel panel-default panel_scheduled"
									id="pnl_l3a_scheduled">

									<div class="row panel-heading">
										<div class="col-md-10">Scheduled Jobs</div>


										<div class="col-md-1">

											<form id="form_add_sched_l3a" method="post" class="form">
												<input class="btn btn-primary " name="schedule_add"
													type="submit" value="AddJob"> <input type="hidden"
													name="processorId" value="2">
											</form>

										</div>
									</div>
									<div class="panel panel-default panel_scheduled_job">

										<!-- l3a processor_id = 2 -->
									<?php
									if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
										if ($_REQUEST ['schedule_add'] == 'AddJob' && $_REQUEST ['processorId'] == '2') {
											
											add_new_scheduled_jobs_layout ( 2 );
										}
									}
									?>
										
									<?php update_scheduled_jobs_layout(2);?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->

						</div>
					</div>
					<!-- L3B Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l3b">
						<a href="#tab_l3b">L3B LAI Processor</a>
						<div>
							<div class="panel_resources_and_output_container"
								id="pnl_l3b_resources_and_output_container">
								<div class="panel_resources_container"
									id="pnl_l3b_resources_container">
									<div class="panel panel-default panel_resources"
										id="pnl_l3b_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container"
									id="pnl_l3b_output_container">
									<div class="panel panel-default panel_output"
										id="pnl_l3b_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container"
								id="pnl_l3b_configuration_container">
								<div class="panel panel-default panel_configuration"
									id="pnl_l3b_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>

							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container"
								id="pnl_l3b_scheduled_container">
								<div class="panel panel-default panel_scheduled"
									id="pnl_l3b_scheduled">

									<div class="row panel-heading">
										<div class="col-md-10">Scheduled Jobs</div>


										<div class="col-md-1">

											<form id="form_add_sched_l3b" method="post">
												<input name="schedule_add" type="submit"
													class="btn btn-primary " value="AddJob"> <input
													type="hidden" name="processorId" value="3">
											</form>

										</div>
									</div>

									<div class="panel panel-default panel_scheduled_job">
										<!-- l3b lai processor_id = 3 -->
									<?php
									
									if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
										if ($_REQUEST ['schedule_add'] == 'AddJob' && $_REQUEST ['processorId'] == '3') {
											
											add_new_scheduled_jobs_layout ( 3 );
										}
									}
									?>
										
									<?php update_scheduled_jobs_layout(3);?>
									
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->

						</div>
					</div>

					<!-- L3b pheno NDVI Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l3b_nvdi">
						<a href="#tab_l3b_nvdi">L3B Pheno Processor</a>
						<div>
							<div class="panel_resources_and_output_container"
								id="pnl_l3b_nvdi_resources_and_output_container">
								<div class="panel_resources_container"
									id="pnl_l3b_nvdi_resources_container">
									<div class="panel panel-default panel_resources"
										id="pnl_l3b_nvdi_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container"
									id="pnl_l3b_nvdi_output_container">
									<div class="panel panel-default panel_output"
										id="pnl_l3b_nvdi_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container"
								id="pnl_l3b_nvdi_configuration_container">
								<div class="panel panel-default panel_configuration"
									id="pnl_l3b_nvdi_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>

							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container"
								id="pnl_l3b_nvdi_scheduled_container">
								<div class="panel panel-default panel_scheduled"
									id="pnl_l3b_nvdi_scheduled">

									<div class="row panel-heading">
										<div class="col-md-10">Scheduled Jobs</div>


										<div class="col-md-1">

											<form id="form_add_sched_l3b_nvdi" method="post">
												<input name="schedule_add" type="submit"
													class="btn btn-primary " value="AddJob"> <input
													type="hidden" name="processorId" value="7">
											</form>

										</div>
									</div>

									<div class="panel panel-default panel_scheduled_job">
										<!-- l3b pheno NDVI processor_id = 7 -->
									<?php
									
									if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
										if ($_REQUEST ['schedule_add'] == 'AddJob' && $_REQUEST ['processorId'] == '7') {
											
											add_new_scheduled_jobs_layout ( 7 );
										}
									}
									?>
										
									<?php update_scheduled_jobs_layout(7);?>
									
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->

						</div>
					</div>


					<!-- L4A Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l4a">
						<a href="#tab_l4a">L4A Processor</a>
						<div>
							<div class="panel_resources_and_output_container"
								id="pnl_l4a_resources_and_output_container">
								<div class="panel_resources_container"
									id="pnl_l4a_resources_container">
									<div class="panel panel-default panel_resources"
										id="pnl_l4a_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container"
									id="pnl_l4a_output_container">
									<div class="panel panel-default panel_output"
										id="pnl_l4a_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>

								<div class="panel-heading">Default Configuration</div>
								<table class="table full_width default_panel_style"></table>

							</div>
							<div class="panel_configuration_container"
								id="pnl_l4a_configuration_container">
								<div class="panel panel-default panel_configuration"
									id="pnl_l4a_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container"
								id="pnl_l4a_scheduled_container">
								<div class="panel panel-default panel_scheduled"
									id="pnl_l4a_scheduled">

									<div class="row panel-heading">
										<div class="col-md-10">Scheduled Jobs</div>


										<div class="col-md-1">

											<form id="form_add_sched" method="post">
												<input name="schedule_add" type="submit"
													class="btn btn-primary " value="AddJob"> <input
													type="hidden" name="processorId" value="4">
											</form>

										</div>
									</div>

									<div class="panel panel-default panel_scheduled_job">
										<!-- l4a processor_id = 4 -->
									<?php
									if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
										if ($_REQUEST ['schedule_add'] == 'AddJob' && $_REQUEST ['processorId'] == '4') {
											
											add_new_scheduled_jobs_layout ( 4 );
										}
									}
									?>
										
									<?php update_scheduled_jobs_layout(4);?>
									</div>
								</div>
							</div>
							<!-- Scheduled ---------------------------->

						</div>
					</div>
					<!-- L4B Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l4b">
						<a href="#tab_l4b">L4B Processor</a>
						<div>
							<div class="panel_resources_and_output_container"
								id="pnl_l4b_resources_and_output_container">
								<div class="panel_resources_container"
									id="pnl_l4b_resources_container">
									<div class="panel panel-default panel_resources"
										id="pnl_l4b_resources">
										<div class="panel-heading">Resource Utilization</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_output_container"
									id="pnl_l4b_output_container">
									<div class="panel panel-default panel_output"
										id="pnl_l4b_output">
										<div class="panel-heading">Output</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
								<div class="panel_scheduled_container"
									id="pnl_l4b_scheduled_container">
									<div class="panel panel-default panel_output"
										id="pnl_l4b_scheduled">
										<div class="panel-heading">Scheduled Tasks</div>
										<table class="table full_width default_panel_style"></table>
									</div>
								</div>
							</div>
							<div class="panel_configuration_container"
								id="pnl_l4b_configuration_container">
								<div class="panel panel-default panel_configuration"
									id="pnl_l4b_configuration">
									<div class="panel-heading">Default Configuration</div>
									<table class="table full_width default_panel_style"></table>
								</div>
							</div>

							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container"
								id="pnl_l4b_scheduled_container">
								<div class="panel panel-default panel_scheduled"
									id="pnl_l4b_scheduled">

									<div class="row panel-heading">
										<div class="col-md-10">Scheduled Jobs</div>


										<div class="col-md-1">

											<form id="form_add_sched" method="post">
												<input name="schedule_add" type="submit"
													class="btn btn-primary " value="AddJob"> <input
													type="hidden" name="processorId" value="5">
											</form>

										</div>
									</div>
									<div class="panel panel-default panel_scheduled_job">

										<!-- l4b processor_id = 5 -->
									<?php
									
									if (isset ( $_REQUEST ['schedule_add'] ) && isset ( $_REQUEST ['processorId'] )) {
										if ($_REQUEST ['schedule_add'] == 'AddJob' && $_REQUEST ['processorId'] == '5') {
											
											add_new_scheduled_jobs_layout ( 5 );
										}
									}
									?>
										
									<?php update_scheduled_jobs_layout(5);?>
									
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

<script src="scripts/config.js"></script>
<script src="scripts/helpers.js"></script>
<script src="scripts/processing_functions.js"></script>
<script src="scripts/processing.js"></script> 


<!-- includes for  datepicker-->
<link rel="stylesheet"
	href="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/themes/smoothness/jquery-ui.css">
<script
	src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
<script
	src="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/jquery-ui.min.js"></script>
<!-- end includes for  datepicker-->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>

<!--Jquery datepicker -->
<script>
		$(document).ready(function() {
			$( ".startdate" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0
					  });
		});
</script>
<!--end Jquery datepicker -->

<!-- Check what job type was selected -->
<script>
		function selectedSchedule(param) {
			var selectedValue = document.getElementById("schedule"+param).value;
			if (selectedValue == "") {
				document.getElementById("div_oneverydate"+param).style.display = "none";
				document.getElementById("div_repeatafter"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "none";
			} else if (selectedValue == "0") {
				document.getElementById("div_oneverydate"+param).style.display = "none";
				document.getElementById("div_repeatafter"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "inline-block";
			} else if (selectedValue == "1") {
				document.getElementById("div_oneverydate"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "inline-block";
				document.getElementById("div_repeatafter"+param).style.display = "inline-block";
			} else if (selectedValue == "2") {
				document.getElementById("div_repeatafter"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "inline-block";
				document.getElementById("div_oneverydate"+param).style.display = "inline-block";
			}
		}
		function selectedScheduleAdd(param) {
			var selectedValue = document.getElementById("schedule_add"+param).value;
			if (selectedValue == "") {
				document.getElementById("div_oneverydate"+param).style.display = "none";
				document.getElementById("div_repeatafter"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "none";
			} else if (selectedValue == "0") {
				document.getElementById("div_oneverydate"+param).style.display = "none";
				document.getElementById("div_repeatafter"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "inline-block";
			} else if (selectedValue == "1") {
				document.getElementById("div_oneverydate"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "inline-block";
				document.getElementById("div_repeatafter"+param).style.display = "inline-block";
			} else if (selectedValue == "2") {
				document.getElementById("div_repeatafter"+param).style.display = "none";
				document.getElementById("div_startdate"+param).style.display = "inline-block";
				document.getElementById("div_oneverydate"+param).style.display = "inline-block";
			}
		}
	</script>
<!--end datepicker -->

<?php include "ms_foot.php"; ?>