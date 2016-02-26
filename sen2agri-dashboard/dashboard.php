<?php include 'master.php'; ?>
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
										<div class="panel-heading">Scheduled Jobs</div>

										<div class="panel_job_container" id="pnl_l2a_job_container">
											<div class="panel panel-default panel_job" id="pnl_l2a_job">

												<div class="panel-heading">Job Name</div>

												<form role="form" id="l2aform" name="l2aform" method="post"
													action="getNewJobConfig.php" target="_blank">

													<div class="form-group form-group-sm div-scheduled">
														<label class="control-label" for="siteName">Site:</label>
													</div>
													<div class="form-group form-group-sm div-scheduled">
														<label class="control-label" for="processorName">Processor:</label>
													</div>
													<div class="form-group form-group-sm div-scheduled">
														<label class="control-label" for="schedule">Schedule:</label> <select
															id="schedule" name="schedule"
															onchange="selectedSchedule()">
															<option value="">Select a schedule</option>
															<option value="Once">Once</option>
															<option value="Cycle">Cycle</option>
															<option value="Repeat">Repeat</option>
														</select>
													</div>
													<div class="form-group form-group-sm div-scheduled"
														id="div_startdate">
														<label class="control-label" for="startdate">Date:</label>
														<input id="startdate" />
													</div>
													<div class="form-group form-group-sm div-scheduled"
														id="div_repeatafter">
														<label class="control-label" for="repeatafter">Repeat
															after:</label> <input id="repeatafter" />
													</div>
													<div class="form-group form-group-sm div-scheduled"
														id="div_oneverydate">
														<label class="control-label" for="oneverydate">On
															every:</label> <input id="oneverydate" />
													</div>
													<div class="form-group form-group-sm div-scheduled">
													<input name="l2a" type="submit" class="btn btn-primary"
														value="SubmitJob">
														</div>
												</form>

											</div>
										</div>
										<!--<table class="table full_width default_panel_style"></table>  -->
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

	<script
		src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-2.1.4.min.js"></script>
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
			$("#oneverydate").datepicker();
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
				document.getElementById("div_startdate").style.display = "block";
			} else if (selectedValue == "Cycle") {
				document.getElementById("div_oneverydate").style.display = "none";
				document.getElementById("div_startdate").style.display = "block";
				document.getElementById("div_repeatafter").style.display = "block";
			} else if (selectedValue == "Repeat") {
				document.getElementById("div_repeatafter").style.display = "none";
				document.getElementById("div_startdate").style.display = "block";
				document.getElementById("div_oneverydate").style.display = "block";
			}
		}
	</script>
	<!--end datepicker -->
<?php include 'ms_foot.php'; ?>