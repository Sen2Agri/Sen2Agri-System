<?php include 'master.php'; ?>
<?php

function dayMonth($param) {
	// keep only day and month, ex:3002
	$date = date ( 'dm', strtotime ( $param ) );
	return $date;
}

// processing insert
if (isset ( $_REQUEST ['add_site'] ) && $_REQUEST ['add_site'] == 'Save Site') {
	// first character to uppercase.
	$site_name = ucfirst ( $_REQUEST ['sitename'] );
	
	// coord geog from the upload file
	// 'POLYGON((35.940406 54.140187, 37.651121 54.140187, 37.651121 53.123645, 35.940406 53.123645, 35.940406 54.140187))'
	$coord_geog = "POLYGON((35.940406 54.140187, 37.651121 54.140187, 37.651121 53.123645, 35.940406 53.123645, 35.940406 54.140187))";
	$winter_start = "";
	$winter_end = "";
	$summer_start = "";
	$summer_end = "";
	function insertSiteSeason($site, $coord, $wint_start, $wint_end, $summ_star, $summ_end) {
		$db = pg_connect ( 'host=' . ConfigParams::$SERVER_NAME . ' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
		$sql = "SELECT sp_dashboard_add_site($1,$2,$3,$4,$5,$6)";
		$res = pg_prepare ( $db, "my_query", $sql );
		
		$res = pg_execute ( $db, "my_query", array (
				$site,
				$coord,
				$wint_start,
				$wint_end,
				$summ_star,
				$summ_end 
		) ) or die ( "An error occurred." );
	}
	
	if (isset ( $_REQUEST ['seasonAdd'] ))
		// winter season selected
		if ($_REQUEST ['seasonAdd'] == "0") {
			$winter_start = dayMonth ( $_REQUEST ['startseason_winter'] );
			$winter_end = dayMonth ( $_REQUEST ['endseason_winter'] );
			insertSiteSeason ( $site_name, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
		} else 
		// summer season selected
		if ($_REQUEST ['seasonAdd'] == "1") {
			$summer_start = dayMonth ( $_REQUEST ['startseason_summer'] );
			$summer_end = dayMonth ( $_REQUEST ['endseason_summer'] );
			insertSiteSeason ( $site_name, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
		} else 
		// summer and winter season selected
		if ($_REQUEST ['seasonAdd'] == "2") {
			$winter_start = dayMonth ( $_REQUEST ['startseason_winter'] );
			$winter_end = dayMonth ( $_REQUEST ['endseason_winter'] );
			$summer_start = dayMonth ( $_REQUEST ['startseason_summer'] );
			$summer_end = dayMonth ( $_REQUEST ['endseason_summer'] );
			
			insertSiteSeason ( $site_name, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
		}
}

// processing edit
if (isset ( $_REQUEST ['edit_site'] ) && $_REQUEST ['edit_site'] == 'Save') {
	
	$site_id = $_REQUEST ['edit_siteid'];
	$shortname = $_REQUEST ['shortname'];
	
	// coord geog from the upload file
	// 'POLYGON((35.940406 54.140187, 37.651121 54.140187, 37.651121 53.123645, 35.940406 53.123645, 35.940406 54.140187))'
	$coord_geog = "POLYGON((35.940406 54.140187, 37.651121 54.140187, 37.651121 53.123645, 35.940406 53.123645, 35.940406 54.140187))";
	function updateSiteSeason($id, $short_name, $coord, $wint_start, $wint_end, $summ_star, $summ_end) {
		$db = pg_connect ( 'host=' . ConfigParams::$SERVER_NAME . ' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
		
		$res = pg_query_params ( $db, "SELECT sp_dashboard_update_site($1,$2,$3,$4,$5,$6,$7)", array (
				$id,
				$short_name,
				$coord,
				$wint_start,
				$wint_end,
				$summ_star,
				$summ_end 
		) ) or die ( "An error occurred." );
	}
	
	$winter_start = dayMonth ( $_REQUEST ['startseason_winterEdit'] );
	$winter_end = dayMonth ( $_REQUEST ['endseason_winterEdit'] );
	$summer_start = dayMonth ( $_REQUEST ['startseason_summerEdit'] );
	$summer_end = dayMonth ( $_REQUEST ['endseason_summerEdit'] );
	
	// echo $winter_start." ".$winter_end." ".$summer_start." ".$summer_end;
	
	updateSiteSeason ( $site_id, $shortname, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
}

?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<!-- Start code for adding site---------- -->
			<div class="panel panel-default">
				<!-- - -->
				<div class="panel-heading">
					<div class="row">
						<div class="col-md-10">Create new site</div>
						<div class="col-md-1">
							<form id="form_add_site" method="post">
								<input name="addsite" type="button" class="btn btn-primary "
									value="Add Site" onclick="formAddSite()">
							</form>
						</div>
					</div>
				</div>
				<!-- - -->

				<div class="panel-body">

					<!---------------------------  form  add site ------------------------>
					<div class="panel panel-default" id="div_addsite"
						style="display: none">
						<div class="panel-body">

							<form enctype="multipart/form-data" id="siteform"
								action="create_site.php" method="post">

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-2">
										<div class="form-group  form-group-sm">
											<label class="control-label" for="sitename">Site name:</label>
											<input type="text" class="form-control" id="sitename"
												name="sitename">
										</div>
										<div class="form-group form-group-sm">
											<label class="control-label" for="seasonAdd"> Season:</label>
											<select id="seasonAdd" class="form-control" name="seasonAdd"
												onchange="displaySeason('seasonAdd')">
												<option value="0" selected>Winter</option>
												<option value="1">Summer</option>
												<option value="2">Winter and Summer</option>
											</select>
										</div>
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-1">

										<div id="div_startseason1" class="form-group form-group-sm">
											<label class="control-label" for="startseason_winter">From:</label>
											<input type="text" class="form-control startseason_winter"
												name="startseason_winter">
										</div>
									</div>

									<div class="col-md-1">
										<div id="div_endseason1" class="form-group form-group-sm">
											<label class="control-label" for="endseason_winter">to:</label>
											<input type="text" class="form-control endseason_winter"
												name="endseason_winter">
										</div>

									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-1">
										<div id="div_startseason2" class="form-group form-group-sm"
											style="display: none">
											<label class="control-label" for="startseason_summer">From:</label>
											<input type="text" class="form-control startseason_summer"
												name="startseason_summer">
										</div>
									</div>
									<div class="col-md-1">
										<div id="div_endseason2" class="form-group form-group-sm"
											style="display: none">
											<label class="control-label" for="endseason_summer">to:</label>
											<input type="text" class="form-control endseason_summer"
												name="endseason_summer">
										</div>
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-2">
										<div class="form-group form-group-sm">
											<label class="control-label" for="zip_file">Upload site shape
												file:</label> <input type="file" class="form-control"
												id="zip_file" name="zip_file">
										</div>
										<input class="btn btn-primary " name="add_site" type="submit"
											value="Save Site">
									</div>
								</div>
							</form>

						</div>
					</div>
					<!---------------------------- end form add ---------------------------------->

					<!---------------------------- form edit sites ------------------------------->
					<div class="panel panel-default" style="display: none"
						id="div_editsite">
						<div class="panel-body">
							<form enctype="multipart/form-data" id="siteform_edit"
								action="create_site.php" method="post">

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-2">

										<div class="form-group  form-group-sm">
											<label class="control-label" for="edit_sitename">Site name:</label>
											<input type="text" class="form-control" id="edit_sitename"
												name="edit_sitename" value="" readonly> <input type="hidden"
												class="form-control" id="edit_siteid" name="edit_siteid"
												value="">
										</div>
									</div>
								</div>
								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-2">
										<div class="form-group  form-group-sm">
											<label class="control-label" for="shortname">Short name:</label>
											<input type="text" class="form-control" id="shortname"
												name="shortname" value="">
										</div>
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-2">
										<div class="form-group form-group-sm">
											<label class="control-label" for="seasonEdit"> Season:</label>
											<select id="seasonEdit" class="form-control"
												name="seasonEdit" onchange="displaySeason('seasonEdit')">
												<option value="0">Winter</option>
												<option value="1">Summer</option>
												<option value="2" selected>Winter and Summer</option>
											</select>
										</div>
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-1">
										<div id="div_startseason1Edit"
											class="form-group form-group-sm">
											<label class="control-label" for="startseason_winterEdit">From:</label>
											<input type="text" class="form-control"
												id="startseason_winterEdit" name="startseason_winterEdit"
												value="">
										</div>
									</div>
									<div class="col-md-1">
										<div id="div_endseason1Edit" class="form-group form-group-sm">
											<label class="control-label" for="endseason_winterEdit">to:</label>
											<input type="text" class="form-control"
												id="endseason_winterEdit" name="endseason_winterEdit"
												value="">
										</div>
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-1">
										<div id="div_startseason2Edit"
											class="form-group form-group-sm">
											<label class="control-label" for="startseason_summerEdit">From:</label>
											<input type="text" class="form-control"
												id="startseason_summerEdit" name="startseason_summerEdit">
										</div>
									</div>
									<div class="col-md-1">
										<div id="div_endseason2Edit" class="form-group form-group-sm">
											<label class="control-label" for="endseason_summerEdit">to:</label>
											<input type="text" class="form-control"
												id="endseason_summerEdit" name="endseason_summerEdit">
										</div>
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-2">
										<div class="form-group form-group-sm">
											<label class="control-label" for="zip_fileEdit">Upload site
												shape file:</label> <input type="file" class="form-control"
												id="zip_fileEdit" name="zip_fileEdit">
										</div>

										<input class="btn btn-primary " name="edit_site" type="submit"
											value="Save">
									</div>
								</div>
							</form>
						</div>
					</div>
					<!------------------------------ end form edit sites -------------------------------->

					<!------------------------------ list of sites -------------------------------------->
					<div class="panel panel-default">
						<div class="panel-body">
							<table class="table table-striped"">
								<thead>
									<tr>
										<th colspan="2">Site name</th>
										<th colspan="2">Short name</th>
										<th colspan="2">Winter season</th>
										<th colspan="2">Summer season</th>
										<th colspan="2"></th>
									</tr>
								</thead>
								<tbody>
								<?php
								function list_sites_seasons($nr_site) {
									$sites = "";
									$count = '1';
									
									$summerSeason = "";
									$winterSeason = "";
									$siteName = "";
									$shortName = "";
									$tr_table = "";
									
									$summerStart = "";
									$summerEnd = "";
									$winterStart = "";
									$winterEnd = "";
									function dayMonthYear($param) {
										$year = date ( 'Y' );
										$date = $year . "-" . $param [0] . "-" . $param [1];
										return $date;
									}
									$sql_select = "";
									$result = "";
									$db = pg_connect ( 'host=' . ConfigParams::$SERVER_NAME . ' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
									
									if ($nr_site == '0') {
										$sql_select = "SELECT * FROM sp_get_dashboard_sites_seasons_2(null)";
										$result = pg_query_params ( $db, $sql_select, array () ) or die ( "Could not execute." );
									} else {
										$sql_select = "SELECT * FROM sp_get_dashboard_sites_seasons_2($1)";
										$result = pg_query_params ( $db, $sql_select, array (
												$_SESSION ['siteId'] 
										) ) or die ( "Could not execute." );
									}
									
									while ( $row = pg_fetch_row ( $result ) ) {
										
										if ($count % 4 == 0) {
											$siteId = $row [0];
											$siteName = $row [1];
											$shortName = $row [2];
											if ($row [3] == 'downloader.summer-season.start') {
												$summerStart = $row [4];
											} elseif ($row [3] == 'downloader.summer-season.end') {
												$summerEnd = $row [4];
											} elseif ($row [3] == 'downloader.winter-season.start') {
												$winterStart = $row [4];
											} elseif ($row [3] == 'downloader.winter-season.end') {
												$winterEnd = $row [4];
											}
											
											// date from MMDD into YYYY-MM-DD
											$summerDate1 = dayMonthYear ( str_split ( $summerStart, 2 ) );
											$summerDate2 = dayMonthYear ( str_split ( $summerEnd, 2 ) );
											$winterDate1 = dayMonthYear ( str_split ( $winterStart, 2 ) );
											$winterDate2 = dayMonthYear ( str_split ( $winterEnd, 2 ) );
											
											$summerSeason = $summerDate1 . ' ' . $summerDate2;
											$winterSeason = $winterDate1 . ' ' . $winterDate2;
											
											$tr_table = "<tr><td colspan=\"2\">" . $siteName . "</td>" . "<td colspan=\"2\">" . $shortName . "</td>" . "<td colspan=\"2\">" . $winterSeason . "</td>" . "<td colspan=\"2\">" . $summerSeason . "</td>" . "<td colspan=\"2\"><a onclick=\"editFormSite('" . $siteId . "','" . $siteName . "','" . $shortName . "','" . $winterDate1 . "','" . $winterDate2 . "','" . $summerDate1 . "','" . $summerDate2 . "')\">Edit</a>" . "</tr>";
											
											$summerSeason = "";
											$winterSeason = "";
											$siteName = "";
											$shortName = "";
										} else {
											if ($row [3] == 'downloader.summer-season.start') {
												$summerStart = $row [4];
											} elseif ($row [3] == 'downloader.summer-season.end') {
												$summerEnd = $row [4];
											} elseif ($row [3] == 'downloader.winter-season.start') {
												$winterStart = $row [4];
											} elseif ($row [3] == 'downloader.winter-season.end') {
												$winterEnd = $row [4];
											}
										}
										
										$sites = $sites . $tr_table;
										$tr_table = "";
										$count ++;
									} // end while
									echo $sites;
								}
								
								if (empty ( $_SESSION ['siteId'] )) {
									//logged as admin, bring all sites
									list_sites_seasons ( 0 );
								} else { // if not admin bring only his site
									list_sites_seasons ( 1 );
									
								}
								?>
								</tbody>
							</table>
						</div>
					</div>
					<!------ list sites ------->

				</div>
			</div>


			<!-- End code for adding site---------- -->
		</div>
	</div>
</div>



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
			//datepickers for form add site
			$( ".startseason_winter" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0,//past dates disable from current date
				 onSelect: function(selected) {
				    $(".endseason_winter").datepicker("option","minDate", selected)
					 	        }
					  });
			$( ".endseason_winter" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0
					  });
			  
			$( ".startseason_summer" ).datepicker({
				 dateFormat: "yy-mm-dd" ,
				 minDate: 0,
				 onSelect: function(selected) {
				    $(".endseason_summer").datepicker("option","minDate", selected)
					 	        }
					  });
			$( ".endseason_summer" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0 
					  });

	//datepickers for forms edit site
			$( "#startseason_winterEdit" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0,//past dates disable from current date
				 onSelect: function(selected) {
				    $("#endseason_winterEdit").datepicker("option","minDate", selected)
					 	        }
					  });
			$( "#endseason_winterEdit" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0
					  });
			  
			$( "#startseason_summerEdit" ).datepicker({
				 dateFormat: "yy-mm-dd" ,
				 minDate: 0,
				 onSelect: function(selected) {
				    $("#endseason_summerEdit").datepicker("option","minDate", selected)
					 	        }
					  });
			$( "#endseason_summerEdit" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0 
					  });
		});
</script>
<!--end Jquery datepicker -->

<!-- validate form  -->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>

<script>
	$(document).ready( function() {
							$("#siteform").validate(
											{
												rules : {
													sitename:"required",						
													startseason_winter : "required",
													endseason_winter:"required",
													startseason_summer:"required",
													endseason_summer:"required",
													zip_file:"required"
												},
												
												// the errorPlacement has to take the table layout into account
												errorPlacement : function(
														error, element) {
													error.appendTo(element
															.parent());
												},

												// set this class to error-labels to indicate valid fields
												success : function(label) {
													label.remove();
												},
												highlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.addClass(
																	"has-error");
												},
												unhighlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.removeClass(
																	"has-error");
												}
											});

							$("#siteform_edit").validate(
									{
										rules : {
											shortname:"required",						
											startseason_winterEdit : "required",
											endseason_winterEdit:"required",
											startseason_summerEdit:"required",
											endseason_summerEdit:"required"
											//zip_fileEdit:"required"
										},
										
										// the errorPlacement has to take the table layout into account
										errorPlacement : function(
												error, element) {
											error.appendTo(element
													.parent());
										},

										// set this class to error-labels to indicate valid fields
										success : function(label) {
											label.remove();
										},
										highlight : function(element,
												errorClass) {
											$(element)
													.parent()
													.addClass(
															"has-error");
										},
										unhighlight : function(element,
												errorClass) {
											$(element)
													.parent()
													.removeClass(
															"has-error");
										}
									});
	});
</script>
<!-- end validate -->

<script>
function formAddSite(){
	document.getElementById("div_editsite").style.display = "none";
	document.getElementById("div_addsite").style.display = "block";
}

function editFormSite(id,name,short_name,winter1,winter2,summer1,summer2){
	document.getElementById("div_addsite").style.display = "none";
	document.getElementById("div_editsite").style.display = "block";
	
	document.getElementById("edit_sitename").value = name;
	document.getElementById("edit_siteid").value = id;
	document.getElementById("shortname").value = short_name;
	
	document.getElementById("startseason_summerEdit").value = summer1;
	document.getElementById("endseason_summerEdit").value = summer2;
	document.getElementById("startseason_winterEdit").value = winter1;
	document.getElementById("endseason_winterEdit").value = winter2;
}

</script>


<!-- Select season event -->
<script type="text/javascript">
function displaySeason(param){
	var elem = document.getElementById(param).value;
	var concat = "";
	if(param =="seasonEdit"){
		concat = "Edit";
	}
	if(elem =="0"){
		document.getElementById("div_startseason1"+concat).style.display = "block";
		document.getElementById("div_endseason1"+concat).style.display = "block";

		document.getElementById("div_startseason2"+concat).style.display = "none";
		document.getElementById("div_endseason2"+concat).style.display = "none";
		}
	else if(elem =="1"){
		document.getElementById("div_startseason1"+concat).style.display = "none";
		document.getElementById("div_endseason1"+concat).style.display = "none";
		
		document.getElementById("div_startseason2"+concat).style.display = "block";
		document.getElementById("div_endseason2"+concat).style.display = "block";
		}
	else if(elem =="2"){
		document.getElementById("div_startseason1"+concat).style.display = "block";
		document.getElementById("div_endseason1"+concat).style.display = "block";
		
		document.getElementById("div_startseason2"+concat).style.display="block";
		document.getElementById("div_endseason2"+concat).style.display="block";
		}
}
</script>
<?php include 'ms_foot.php'; ?>