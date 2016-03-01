<?php include 'master.php';?>
<?php
if (isset ( $_POST ['schedule_edit'] )) {
	$db = pg_connect ( 'host=sen2agri-dev port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );

	$schedule_id = $_REQUEST ['scheduledID'];
	$schedule_type = $_REQUEST ['schedule'];
	$startdate = $_REQUEST ['startdate'];
	$repeatafter = $_REQUEST ['repeatafter'];
	$oneverydate = $_REQUEST ['oneverydate'];
	echo $startdate;
	//$startdate="2015-02-28";
	//in baza de date adauga "1985" ???
	if($schedule_type =='0'){
		$sql = "UPDATE scheduled_task ".
				"SET repeat_type = ".$schedule_type.", first_run_time = ".$startdate." ".
				"WHERE id=".$schedule_id."";
	} elseif ($schedule_type =='1'){
		$sql = "UPDATE scheduled_task ".
				"SET repeat_type = ".$schedule_type.",".
				"first_run_time = ".$startdate.",".
				"repeat_after_days = ".$repeatafter." ".
				"WHERE id=$schedule_id";
	}else{
	
		$sql = "UPDATE scheduled_task ".
			"SET repeat_type = ".$schedule_type.",".
			"first_run_time = ".$startdate.",".
			"repeat_after_days = ".$repeatafter.",".
			"repeat_on_month_day = ".$oneverydate."".
			"WHERE id=$schedule_id";
	}
	//echo $sql;
	//$result = pg_query($db,$sql) or die("Could not execute.");
}
?>
<?php include 'dashboardScript.php';?>


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
							<!-- Scheduled ---------------------------->
							<div class="panel_scheduled_container"
								id="pnl_l2a_scheduled_container">
								<div class="panel panel-default panel_scheduled"
									id="pnl_l2a_scheduled">
									<div class="panel-heading">Scheduled Jobs
									<input  class="right" name="addjob" type="submit"class="btn btn-primary" value="AddJob"></div>
									<!-- l2a processor_id = 1 -->
									<?php update_scheduled_jobs_layout(2);?>
								</div>
							</div>
							<!-- Scheduled ---------------------------->
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
									<div class="panel-heading">Scheduled Jobs</div>
									<!-- l3a processor_id = 2 -->
									<?php update_scheduled_jobs_layout(2)?>

								</div>
							</div>
							<!-- Scheduled ---------------------------->

						</div>
					</div>
					<!-- L3B Processor ----------------------------------------------------------------------------------------------------------------------- -->
					<div id="tab_l3b">
						<a href="#tab_l3b">L3B Processor</a>
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
									<div class="panel-heading">Scheduled Jobs</div>
									<!-- l3b lai processor_id = 3 -->
									<?php update_scheduled_jobs_layout(3)?>

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
									<div class="panel-heading">Scheduled Jobs</div>
									<!-- l4a processor_id = 4 -->
									<?php update_scheduled_jobs_layout(4)?>

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
									<div class="panel-heading">Scheduled Jobs</div>
									<!-- l4b processor_id = 5 -->
									<?php update_scheduled_jobs_layout(5)?>

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
<!-- main -->
<!-- main2 -->
<!-- main3 -->

<script src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-2.1.4.min.js"></script>
<!--<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>-->
<script src="https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.6/d3.min.js"></script>
<script src="libraries/flot-0.8.3/jquery.flot.min.js"></script>
<script src="libraries/flot-0.8.3/jquery.flot.time.min.js"></script>
<script src="libraries/flot-0.8.3/jquery.flot.stack.min.js"></script>
<script src="libraries/nvd3-1.1.11/nv.d3.js"></script>
<script src="scripts/config.js"></script>
<script src="scripts/helpers.js"></script>
<script src="scripts/processing_functions.js"></script>
<!-- <script src="scripts/processing.js"></script> -->
<script src="scripts/dashoboardScript.js"></script>

<!-- includes for  datepicker-->
<link rel="stylesheet"
	href="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/themes/smoothness/jquery-ui.css">
<script
	src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
<script
	src="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/jquery-ui.min.js"></script>
<!-- end includes for  datepicker-->

<!--Jquery datepicker -->
<script>
		$(document).ready(function() {
			$("#startdate").datepicker();
			//$("#oneverydate").datepicker();
		});
	</script>
<!--end Jquery datepicker -->

<!-- Check what job type was selected -->
<script>
		function selectedSchedule() {
			var selectedValue = document.getElementById("schedule").value;
			if (selectedValue == "") {
				document.getElementById("div_startdate").style.display = "none";
				document.getElementById("div_repeatafter").style.display = "none";
				document.getElementById("div_startdate").style.display = "none";
			} else if (selectedValue == "Once") {
				document.getElementById("div_startdate").style.display = "none";
				document.getElementById("div_repeatafter").style.display = "none";
				document.getElementById("div_startdate").style.display = "inline";
			} else if (selectedValue == "Cycle") {
				document.getElementById("div_oneverydate").style.display = "none";
				document.getElementById("div_startdate").style.display = "inline";
				document.getElementById("div_repeatafter").style.display = "inline";
			} else if (selectedValue == "Repeat") {
				document.getElementById("div_repeatafter").style.display = "none";
				document.getElementById("div_startdate").style.display = "inline";
				document.getElementById("div_oneverydate").style.display = "inline";
			}
		}
	</script>
<!--end datepicker -->

<?php include 'ms_foot.php'; ?>