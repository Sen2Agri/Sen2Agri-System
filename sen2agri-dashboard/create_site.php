<?php include 'master.php'; ?>
<?php

function endsWith($str, $sub) {
	return (substr ( $str, strlen ( $str ) - strlen ( $sub ) ) === $sub);
}
function upload_reference_polygons() {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	
	$rows = pg_query ( $db, "SELECT key, value FROM sp_get_parameters('site.upload_path') WHERE site_id IS NULL" ) or die ( pg_last_error () );
	$result = pg_fetch_array ( $rows, 0 ) [1];
	
	$upload_target_dir = str_replace ( "{user}", "", $result );
	$upload_target_dir = $upload_target_dir . $_SESSION ['userName'] . "/";
	
	if (! is_dir ( $upload_target_dir )) {
		mkdir ( $upload_target_dir, 0755, true );
	}
	
	$zip_msg = '';
	$shp_file = false;
	if ($_FILES ["zip_file"] ["name"]) {
		$filename = $_FILES ["zip_file"] ["name"];
		$source = $_FILES ["zip_file"] ["tmp_name"];
		$type = $_FILES ["zip_file"] ["type"];
		
		$zip_file = false;
		$accepted_types = array (
				'application/zip',
				'application/x-zip-compressed',
				'multipart/x-zip',
				'application/x-compressed' 
		);
		foreach ( $accepted_types as $mime_type ) {
			if ($mime_type == $type) {
				$zip_file = true;
				break;
			}
		}
		if ($zip_file) {
			$target_path = $upload_target_dir . $filename;
			if (move_uploaded_file ( $source, $target_path )) {
				$zip = new ZipArchive ();
				$x = $zip->open ( $target_path );
				if ($x === true) {
					for($i = 0; $i < $zip->numFiles; $i ++) {
						$filename = $zip->getNameIndex ( $i );
						if (endsWith ( $filename, '.shp' )) {
							$shp_file = $upload_target_dir . $filename;
							break;
						}
					}
					$zip->extractTo ( $upload_target_dir );
					$zip->close ();
					unlink ( $target_path );
					if ($shp_file) {
						$zip_msg = "Your .zip file was uploaded and unpacked successfully.";
					} else {
						$zip_msg = "Your .zip file does not contain any shape (.shp) file.";
					}
				} else {
					$zip_msg = "Your file is not a valid .zip archive.";
				}
			} else {
				$zip_msg = "Failed to upload the file you selected.";
			}
		} else {
			$zip_msg = "The file you selected is not a .zip file.";
		}
	} else {
		$zip_msg = 'Unable to access your selected file.';
	}
	
	// verify if shape file has valid geometry
	$shp_msg = '';
	$shape_ok = false;
	if ($shp_file) {
		exec ( 'scripts/check_shp.py ' . $shp_file, $output, $ret );
		
		if ($ret === FALSE) {
			$shp_msg = 'Invalid command line.';
		} else {
			switch ($ret) {
				case 0 :
					$shape_ok = true;
					break;
				case 1 :
					$shp_file = false;
					$shp_msg = 'Unable to open the shape file.';
					break;
				case 2 :
					$shp_file = false;
					$shp_msg = 'Shape file has invalid geometry.';
					break;
				case 127 :
					$shp_file = false;
					$shp_msg = 'Invalid geometry detection script.';
					break;
				default :
					$shp_file = false;
					$shp_msg = 'Unexpected error with the geometry detection script.';
					break;
			}
		}
		if ($shape_ok) {
			$last_line = $output [count ( $output ) - 1];
			$r = preg_match ( '/^Union: (.+)$/m', $last_line, $matches );
			if (! $r) {
				$shp_file = false;
				$shp_msg = 'Unable to parse shape';
			} else {
				$shp_msg = $matches [1];
			}
		}
	} else {
		$shp_msg = 'Missing shape file due to a problem with your selected file.';
	}
	
	return array (
			"polygons_file" => $shp_file,
			"result" => $shp_msg,
			"message" => $zip_msg 
	);
}// end funcion upload

function dayMonth($param) {
	// keep only month and day(0230)
	$date = date ( 'md', strtotime ( $param ) );
	return $date;
}

// processing insert
if (isset ( $_REQUEST ['add_site'] ) && $_REQUEST ['add_site'] == 'Save Site') {
	// first character to uppercase.
	$site_name = ucfirst ( $_REQUEST ['sitename'] );
	
	// upload polygons
	$upload = upload_reference_polygons ();
	$polygons_file = $upload ['polygons_file'];
	$coord_geog = $upload ['result'];
	$message = $upload ['message'];
	
	$winter_start = "";
	$winter_end = "";
	$summer_start = "";
	$summer_end = "";
	
	function insertSiteSeason($site, $coord, $wint_start, $wint_end, $summ_star, $summ_end) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
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
	
	if ($polygons_file) {
		
		if (isset ( $_REQUEST ['seasonAdd'] )) {
			// winter season selected
			if ($_REQUEST ['seasonAdd'] == "0") {
				$winter_start = dayMonth ( $_REQUEST ['startseason_winter'] );
				$winter_end = dayMonth ( $_REQUEST ['endseason_winter'] );
				insertSiteSeason ( $site_name, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
				
				$_SESSION['status'] =  "OK"; $_SESSION['message'] = "Your site has been successfully added!";
			} else 
			// summer season selected
			if ($_REQUEST ['seasonAdd'] == "1") {
				$summer_start = dayMonth ( $_REQUEST ['startseason_summer'] );
				$summer_end = dayMonth ( $_REQUEST ['endseason_summer'] );
				insertSiteSeason ( $site_name, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
				
				$_SESSION['status'] =  "OK"; $_SESSION['message'] = "Your site has been successfully added!";
			} else 
			// summer and winter season selected
			if ($_REQUEST ['seasonAdd'] == "2") {
				$winter_start = dayMonth ( $_REQUEST ['startseason_winter'] );
				$winter_end = dayMonth ( $_REQUEST ['endseason_winter'] );
				$summer_start = dayMonth ( $_REQUEST ['startseason_summer'] );
				$summer_end = dayMonth ( $_REQUEST ['endseason_summer'] );
				
				insertSiteSeason ( $site_name, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
				$_SESSION['status'] =  "OK"; $_SESSION['message'] = "Your site has been successfully added!";
			}
		}
	} else {
		$_SESSION['status'] =  "NOK"; $_SESSION['message'] = $message;  $_SESSION['result'] = $coord_geog;

	}
}

// processing edit
if (isset ( $_REQUEST ['edit_site'] ) && $_REQUEST ['edit_site'] == 'Save') {
	
	$site_id = $_REQUEST ['edit_siteid'];
	$shortname = $_REQUEST ['shortname'];
	
	// upload polygons
	$upload = upload_reference_polygons ();
	$polygons_file = $upload ['polygons_file'];
	$coord_geog = $upload ['result'];
	$message = $upload ['message'];
	
	function updateSiteSeason($id, $short_name, $coord, $wint_start, $wint_end, $summ_star, $summ_end) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
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
		
	if ($polygons_file) {
		
		updateSiteSeason ( $site_id, $shortname, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end );
		$_SESSION['status'] =  "OK"; $_SESSION['message'] = "Your site has been successfully modified!";
	} else {
		$_SESSION['status'] =  "NOK"; $_SESSION['message'] = $message; $_SESSION['result'] = $coord_geog;
	}

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
					<?php if (! (empty ( $_SESSION ['siteId'] ))) { // not admin
						?>
					<div class="col-md-10">Seasons site details</div>
						<div class="col-md-1">
						</div>
					</div>
				<?php 	}else {// if admin
					?>
						<div class="col-md-10">Create new site</div>
						<div class="col-md-1">
							<form id="form_add_site" method="post">
								<input name="addsite" type="button" class="btn btn-primary "
									value="Add Site" onclick="formAddSite()">
							</form>
						</div>
					</div>
					<?php } ?>
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
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-1">
										<input class="btn btn-primary " name="add_site" type="submit"
											value="Save Site">
									</div>
									<div class="col-md-1">
										<input class="btn btn-primary " name="abort_add" type="button"
											value="Abort" onclick="abortEditAdd('add')">


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
												id="zip_fileEdit" name="zip_file">
										</div>
									</div>
								</div>

								<div class="row">
									<div class="col-md-1"></div>
									<div class="col-md-1">
										<input class="btn btn-primary " name="edit_site" type="submit"
											value="Save">
									</div>
									<div class="col-md-1">
										<input class="btn btn-primary " name="abort_edit"
											type="button" value="Abort" onclick="abortEditAdd('edit')">
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
									$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
									
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
											$summerDate1 =  str_split ( $summerStart, 2 ) ;
											$summerDate2 =  str_split ( $summerEnd, 2 ) ;
											$winterDate1 =  str_split ( $winterStart, 2 ) ;
											$winterDate2 =  str_split ( $winterEnd, 2 ) ;
											
											if($summerDate2[0]<$summerDate1[0]){
												$summer2 =( date ( 'Y' )+1). "-" . $summerDate2 [0] . "-" . $summerDate2 [1];
											}else{
												$summer2 = dayMonthYear($summerDate2);
											}
											$summer1 = dayMonthYear($summerDate1);
											
											if($winterDate2[0]<$winterDate1[0]){
												$winter2 =( date ( 'Y' )+1). "-" . $winterDate2 [0] . "-" . $winterDate2 [1];
											}else{
												$winter2 = dayMonthYear($winterDate2);
											}
											$winter1 = dayMonthYear($winterDate1);
											
											$summerSeason = $summer1 . ' ' . $summer2;
											$winterSeason = $winter1 . ' ' . $winter2;
											
											$tr_table = "<tr><td colspan=\"2\">" . $siteName . "</td>" . "<td colspan=\"2\">" . $shortName . "</td>" . "<td colspan=\"2\">" . $winterSeason . "</td>" . "<td colspan=\"2\">" . $summerSeason . "</td>" . "<td colspan=\"2\"><a onclick=\"editFormSite('" . $siteId . "','" . $siteName . "','" . $shortName . "','" . $winter1 . "','" . $winter2 . "','" . $summer1 . "','" . $summer2 . "')\">Edit</a>" . "</tr>";
											
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
									// logged as admin, get all sites
									list_sites_seasons ( 0 );
								} else { // if not admin get only his site
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

<!-- validate form  -->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>

<script>
/*
function message_alert(message){
	alert(message);
}*/
	$(document).ready( function() {
		//datepickers for form add site
		 $(".startseason_winter").datepicker({
			  dateFormat: "yy-mm-dd",
		        minDate: 0,
		        onSelect: function (date) {
		            var date2 = $('.startseason_winter').datepicker('getDate');
		                date2.setDate(date2.getDate() + 1);
		            $('.endseason_winter').datepicker('setDate', date2);
		            //sets minDate to dt1 date + 1
		            $('.endseason_winter').datepicker('option', 'minDate', date2);
		        }
		    });
		    $('.endseason_winter').datepicker({
		    	 dateFormat: "yy-mm-dd",
		    	 minDate: 0,
		        onClose: function () {
		            var dt1 = $('.startseason_winter').datepicker('getDate');
		            var dt2 = $('.endseason_winter').datepicker('getDate');
		            //check to prevent a user from entering a date below date of dt1
		            if (dt2 <= dt1) {
		                var minDate = $('.startseason_winter').datepicker('option', 'minDate');
		                $('.endseason_winter').datepicker('setDate', minDate);
		            }
		        }
		    });

		  $(".startseason_summer").datepicker({
			  dateFormat: "yy-mm-dd",
		        minDate: 0,
		        onSelect: function (date) {
		            var date2 = $('.startseason_summer').datepicker('getDate');
		            date2.setDate(date2.getDate() + 1);
		            $('.endseason_summer').datepicker('setDate', date2);
		            //sets minDate to dt1 date + 1
		            $('.endseason_summer').datepicker('option', 'minDate', date2);
		        }
		    });
		    $('.endseason_summer').datepicker({
		    	 dateFormat: "yy-mm-dd",
		    	 minDate: 0,
		        onClose: function () {
		            var dt1 = $('.startseason_summer').datepicker('getDate');
		            var dt2 = $('.endseason_summer').datepicker('getDate');
		            //check to prevent a user from entering a date below date of dt1
		            if (dt2 <= dt1) {
		                var minDate = $('.startseason_summer').datepicker('option', 'minDate');
		                $('.endseason_summer').datepicker('setDate', minDate);
		            }
		        }
		    });

//datepickers for form edit site
		$( "#startseason_winterEdit" ).datepicker({
			  dateFormat: "yy-mm-dd",
		        minDate: 0,
		        onSelect: function (date) {
		         var date2 = $('#startseason_winterEdit').datepicker('getDate');
			         date2.setDate(date2.getDate() + 1);
			      $('#endseason_winterEdit').datepicker('setDate', date2);
			            //sets minDate to dt1 date + 1
			            $('#endseason_winterEdit').datepicker('option', 'minDate', date2);
			            
		        }
		    });
		    $("#endseason_winterEdit").datepicker({
		    	 dateFormat: "yy-mm-dd",
		    	 minDate: 0,
		        onClose: function () {
		            var dt1 = $("#startseason_winterEdit").datepicker('getDate');
		            var dt2 = $("#endseason_winterEdit").datepicker('getDate');
		            //check to prevent a user from entering a date below date of dt1
		            if (dt2 <= dt1) {
		                var minDate = $("#startseason_winterEdit").datepicker('option', 'minDate');
		                $("#endseason_winterEdit").datepicker('setDate', minDate);
		            }
		        }
		    });

			  $("#startseason_summerEdit").datepicker({
				  dateFormat: "yy-mm-dd",
			        minDate: 0,
			        onSelect: function (date) {
			            var date2 = $("#startseason_summerEdit").datepicker('getDate');
			            date2.setDate(date2.getDate() + 1);
			            $("#endseason_summerEdit").datepicker('setDate', date2);
			            //sets minDate to dt1 date + 1
			            $("#endseason_summerEdit").datepicker('option', 'minDate', date2);
			        }
			    });
			    $('#endseason_summerEdit').datepicker({
			    	 dateFormat: "yy-mm-dd",
			    	 minDate: 0,
			        onClose: function () {
			            var dt1 = $("#startseason_summerEdit").datepicker('getDate');
			            var dt2 = $("#endseason_summerEdit").datepicker('getDate');
			            //check to prevent a user from entering a date below date of dt1
			            if (dt2 <= dt1) {
			                var minDate = $("#startseason_summerEdit").datepicker('option', 'minDate');
			                $("#endseason_summerEdit").datepicker('setDate', minDate);
			            }
			        }
			    });

		         <?php if (isset($_SESSION['status'])){
		         			if ( $_SESSION['status'] =='OK') { 
		         				echo "alert('".$_SESSION['message']."')";
		         				unset($_SESSION['status']);
		         				unset($_SESSION['message']);
		         			}else if($_SESSION['status']=='NOK' && isset($_SESSION['result'])){
		         				echo "alert('FAILED:".$_SESSION['message']." ".$_SESSION['result'] ."')";
		         				unset($_SESSION['status']);
		         				unset($_SESSION['message']);
		         				unset($_SESSION['result']);
		         	
		       	 			 }
		        	 }
		         	?>

			    //validate form add site
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
											
									highlight: function(element, errorClass) {
										$(element).parent().addClass("has-error");
									},
									unhighlight: function(element, errorClass) {
										$(element).parent().removeClass("has-error");
									},
									errorPlacement: function(error, element) {
										error.appendTo(element.parent());
									},
									submitHandler: function(form) {
										$.ajax({
											url: $(form).attr('action'),
											type: $(form).attr('method'),
											data: new FormData(form),
											success: function(response) {
											}
										 });
									},
									// set this class to error-labels to indicate valid fields
									success: function(label) {
										label.remove();
									},
								});	

			$("#siteform_edit").validate(
								{
									rules : {
										shortname:"required",						
										startseason_winterEdit : "required",
										endseason_winterEdit:"required",
										startseason_summerEdit:"required",
										endseason_summerEdit:"required",
										zip_file:"required"
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
									submitHandler: function(form) {
										$.ajax({
											url: $(form).attr('action'),
											type: $(form).attr('method'),
											data: new FormData(form),
											success: function(response) {}
										 });
									},
									// set this class to error-labels to indicate valid fields
									success: function(label) {
										label.remove();
									},
								});	
			  
	});
</script>
<!-- end validate -->

<script>
function formAddSite(){
	document.getElementById("div_editsite").style.display = "none";
	document.getElementById("div_addsite").style.display = "block";
	$("#siteform")[0].reset();
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

function abortEditAdd(abort){
	if(abort=='edit'){
		document.getElementById("div_editsite").style.display = "none";
		}else if(abort=='add')
	document.getElementById("div_addsite").style.display = "none";
	$("#siteform")[0].reset();
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