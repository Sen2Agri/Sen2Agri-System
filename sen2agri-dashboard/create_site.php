<?php include 'master.php'; ?>
<?php

function endsWith($str, $sub) {
	return (substr ( $str, strlen ( $str ) - strlen ( $sub ) ) === $sub);
}

function createCustomUploadFolder($siteId, $timestamp) {
	// create custom upload path like: /mnt/upload/siteName/userName_timeStamp/
	$dbconn = pg_connect( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$rows = pg_query($dbconn, "SELECT key, value FROM sp_get_parameters('site.upload_path') WHERE site_id IS NULL") or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[1];
	$upload_target_dir = str_replace("{user}", "", $result);
	$result = $siteId;
	if (is_numeric($siteId)) {
		$rows = pg_query($dbconn, "SELECT name FROM sp_get_sites() WHERE id = ".$siteId) or die(pg_last_error());
		$result = pg_fetch_array($rows, 0)[0];
	}
	$upload_target_dir = $upload_target_dir . $result . "/" . ConfigParams::$USER_NAME . "_".$timestamp . "/";
	$upload_target_dir = str_replace("(", "", $upload_target_dir);
	$upload_target_dir = str_replace(")", "", $upload_target_dir);
	$upload_target_dir = str_replace(" ", "_", $upload_target_dir);
	if (!is_dir($upload_target_dir)) {
		mkdir($upload_target_dir, 0755, true);
	}
	return $upload_target_dir;
}

function uploadReferencePolygons($zipFile, $siteId, $timestamp) {
	$zip_msg = "";
	$shp_file = false;
	if ($_FILES[$zipFile]["name"]) {
		$filename = $_FILES[$zipFile]["name"];
		$source = $_FILES[$zipFile]["tmp_name"];

        $upload_target_dir = createCustomUploadFolder($siteId, $timestamp);

        $target_path = $upload_target_dir . $filename;
        if(move_uploaded_file($source, $target_path)) {
            $zip = new ZipArchive();
            $x = $zip->open($target_path);
            if ($x === true) {
                for ($i = 0; $i < $zip->numFiles; $i++) {
                    $filename = $zip->getNameIndex($i);
                    if (endsWith($filename, '.shp')) {
                        $shp_file = $upload_target_dir . $filename;
                        break;
                    }
                }
                $zip->extractTo($upload_target_dir);
                $zip->close();
                unlink($target_path);
                if ($shp_file) {
                    $zip_msg = "Your .zip file was uploaded and unpacked successfully";
                } else {
                    $zip_msg = "Your .zip file does not contain any shape (.shp) file";
                }
            } else {
                $zip_msg = "Your file is not a valid .zip archive";
            }
        } else {
            $zip_msg = "Failed to upload the file you selected";
        }
	} else {
		$zip_msg = 'Unable to access your selected file';
	}

	// verify if shape file has valid geometry
	$shp_msg = '';
	$shape_ok = false;
	if ($shp_file) {
		exec('scripts/check_shp.py -b '.$shp_file, $output, $ret);

		if ($ret === FALSE) {
			$shp_msg = 'Invalid command line';
		} else {
			switch ($ret) {
				case 0:     $shape_ok = true; break;
				case 1:     $shp_file = false; $shp_msg = 'Unable to open the shape file'; break;
				case 2:     $shp_file = false; $shp_msg = 'Shape file has invalid geometry'; break;
				case 3:		$shp_file = false; $shp_msg = 'Shape file has overlapping polygons'; break;
				case 127:   $shp_file = false; $shp_msg = 'Invalid geometry detection script'; break;
				default:    $shp_file = false; $shp_msg = 'Unexpected error with the geometry detection script'; break;
			}
		}
		if ($shape_ok) {
			$last_line = $output[count($output) - 1];
			$r = preg_match('/^Union: (.+)$/m', $last_line, $matches);
			if (!$r) {
				$shp_file = false;
				$shp_msg = 'Unable to parse shape';
			} else {
				$shp_msg = $matches[1];
			}
		}
	} else {
		$shp_msg = 'Missing shape file due to a problem with your selected file';
	}

	return array ( "polygons_file" => $shp_file, "result" => $shp_msg, "message" => $zip_msg );
}

function dayMonthYear($param) {
	// keep only month and day(0230)
	$date = date ( 'Ymd', strtotime ( $param ) );
	return $date;
}

function dayMonth($param) {
	// keep only month and day(0230)
	$date = date ( 'md', strtotime ( $param ) );
	return $date;
}

// processing add site
if (isset ( $_REQUEST ['add_site'] ) && $_REQUEST ['add_site'] == 'Save New Site') {
	// first character to uppercase.
	$site_name = ucfirst ( $_REQUEST ['sitename'] );
	$site_enabled = empty($_REQUEST ['add_enabled']) ? "0" : "1";
	$winter_start = "";
	$winter_end = "";
	$summer_start = "";
	$summer_end = "";

	function insertSiteSeason($site, $coord, $wint_start, $wint_end, $summ_star, $summ_end, $enbl) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$sql = "SELECT sp_dashboard_add_site($1,$2,$3,$4,$5,$6,$7)";

		$res = pg_prepare ( $db, "my_query", $sql );

		$res = pg_execute ( $db, "my_query", array (
				$site,
				$coord,
				$wint_start,
				$wint_end,
				$summ_star,
				$summ_end,
				$enbl
		) ) or die ( "An error occurred." );
	}

	$date = date_create();
	$time_stamp = date_timestamp_get($date);

	// upload polygons
	$upload = uploadReferencePolygons("zip_fileAdd", $site_name, $time_stamp);
	$polygons_file = $upload ['polygons_file'];
	$coord_geog = $upload ['result'];
	$message = $upload ['message'];
	if ($polygons_file) {
		switch ($_REQUEST ['seasonAdd']) {
			case "0": // Winter season selected
				$winter_start = dayMonthYear ($_REQUEST['startseason_winter'] );
				$winter_end   = dayMonthYear ($_REQUEST['endseason_winter'] );
				break;
			case "1": // Summer season selected
				$summer_start = dayMonthYear ($_REQUEST['startseason_summer'] );
				$summer_end   = dayMonthYear ($_REQUEST['endseason_summer'] );
				break;
			case "2": // Winter and Summer season selected
				$winter_start = dayMonthYear ($_REQUEST['startseason_winter'] );
				$winter_end   = dayMonthYear ($_REQUEST['endseason_winter'] );
				$summer_start = dayMonthYear ($_REQUEST['startseason_summer'] );
				$summer_end   = dayMonthYear ($_REQUEST['endseason_summer'] );
				break;
		}
		insertSiteSeason ( $site_name, $coord_geog, $winter_start, $winter_end, $summer_start, $summer_end, $site_enabled );
		$_SESSION['status'] =  "OK"; $_SESSION['message'] = "Your site has been successfully added!";
	} else {
		$_SESSION['status'] =  "NOK"; $_SESSION['message'] = $message;  $_SESSION['result'] = $coord_geog;
	}

	// Prevent adding site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}

// processing edit site
if (isset ( $_REQUEST ['edit_site'] ) && $_REQUEST ['edit_site'] == 'Save Site') {
	$site_id      = $_REQUEST ['edit_siteid'];
	$shortname    = $_REQUEST ['shortname'];
	$site_enabled = empty($_REQUEST ['edit_enabled']) ? "0" : "1";
	$winter_start = "";
	$winter_end = "";
	$summer_start = "";
	$summer_end = "";

	function polygonFileSelected($name) {
		foreach($_FILES as $key => $val){
			if (($key == $name) && (strlen($_FILES[$key]['name'])) > 0) {
				return true;
			}
		}
		return false;
	}

	function updateSiteSeason($id, $short_name, $coord, $wint_start, $wint_end, $summ_star, $summ_end, $enbl) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$res = pg_query_params ( $db, "SELECT sp_dashboard_update_site($1,$2,$3,$4,$5,$6,$7,$8)", array (
				$id,
				$short_name,
				$coord,
				$wint_start,
				$wint_end,
				$summ_star,
				$summ_end,
				$enbl
		) ) or die ( "An error occurred." );
	}

	$date = date_create();
	$time_stamp = date_timestamp_get($date);

	// upload polygons if zip file selected
	$shape_file = null;
	$status     = "OK";
	$message    = "";
	if (polygonFileSelected("zip_fileEdit")) {
		$upload        = uploadReferencePolygons("zip_fileEdit", $site_id, $time_stamp);
		$polygons_file = $upload ['polygons_file'];
		$coord_geog    = $upload ['result'];
		$message       = $upload ['message'];
		if ($polygons_file) {
			$message = "Your site has been successfully modified!";
			$shape_file = $coord_geog;
		} else {
			$status = "NOK";
			$_SESSION['result'] = $coord_geog;
		}
	} else {
		$message = "Your site has been successfully modified!";
	}
	if ($status == "OK") {
		switch ($_REQUEST ['seasonEdit']) {
			case "0": // Winter season selected
				$winter_start = dayMonthYear ($_REQUEST ['startseason_winterEdit'] );
				$winter_end   = dayMonthYear ($_REQUEST ['endseason_winterEdit'] );
				break;
			case "1": // Summer season selected
				$summer_start = dayMonthYear ($_REQUEST ['startseason_summerEdit'] );
				$summer_end   = dayMonthYear ($_REQUEST ['endseason_summerEdit'] );
				break;
			case "2": // Winter and Summer season selected
				$winter_start = dayMonthYear ($_REQUEST ['startseason_winterEdit'] );
				$winter_end   = dayMonthYear ($_REQUEST ['endseason_winterEdit'] );
				$summer_start = dayMonthYear ($_REQUEST ['startseason_summerEdit'] );
				$summer_end   = dayMonthYear ($_REQUEST ['endseason_summerEdit'] );
				break;
		}
		updateSiteSeason($site_id, $shortname, $shape_file, $winter_start, $winter_end, $summer_start, $summer_end, $site_enabled);
	}
	$_SESSION['status'] =  $status; $_SESSION['message'] = $message;

	// Prevent updating site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}
?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<!-- Start code for adding site---------- -->
			<div class="panel panel-default create-site">
				<div class="panel-body">
<?php
if (!(empty($_SESSION ['siteId']))) {
	// not admin ?>
					<div class="panel-heading row">Seasons site details</div>
<?php
} else {
	// admin ?>
					<div class="panel-heading row">
						<input name="addsite" type="button" class="add-edit-btn" value="Create new site" onclick="formAddSite()" style="width: 200px">
					</div>
<?php
} ?>
					<!-- - -->

					<!---------------------------  form  add site ------------------------>
					<div class="add-edit-site" id="div_addsite" style="display: none;">
						<form enctype="multipart/form-data" id="siteform" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-1">
									<div class="form-group  form-group-sm">
										<label class="control-label" for="sitename">Site name:</label>
										<input type="text" class="form-control" id="sitename" name="sitename">
									</div>
									<div class="form-group form-group-sm">
										<label class="control-label" for="seasonAdd"> Season:</label>
										<select id="seasonAdd" class="form-control" name="seasonAdd" onchange="displaySeason('seasonAdd')">
											<option value="0" selected>Winter</option>
											<option value="1">Summer</option>
											<option value="2">Winter and Summer</option>
										</select>
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-2">

									<div id="div_startseason1" class="form-group form-group-sm">
										<label class="control-label" for="startseason_winter">Winter from:</label>
										<input type="text" class="form-control" id="startseason_winter" name="startseason_winter">
									</div>
								</div>
								<div class="col-md-2">
									<div id="div_endseason1" class="form-group form-group-sm">
										<label class="control-label" for="endseason_winter">Winter to:</label>
										<input type="text" class="form-control" id="endseason_winter" name="endseason_winter">
									</div>

								</div>
							</div>
							<div class="row">
								<div class="col-md-2">
									<div id="div_startseason2" class="form-group form-group-sm" style="display: none">
										<label class="control-label" for="startseason_summer">Summer from:</label>
										<input type="text" class="form-control" id="startseason_summer" name="startseason_summer">
									</div>
								</div>
								<div class="col-md-2">
									<div id="div_endseason2" class="form-group form-group-sm" style="display: none">
										<label class="control-label" for="endseason_summer">Summer to:</label>
										<input type="text" class="form-control" id="endseason_summer" name="endseason_summer">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="zip_fileAdd">Upload site shape file:</label>
										<input type="file" class="form-control" id="zip_fileAdd" name="zip_fileAdd">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="add_enabled">Enable site:</label>
										<input type="checkbox" id="add_enabled" name="add_enabled" checked>
									</div>
								</div>
							</div>
							<div class="submit-buttons">
								<input class="add-edit-btn" name="add_site" type="submit" value="Save New Site">
								<input class="add-edit-btn" name="abort_add" type="button" value="Abort" onclick="abortEditAdd('add')">
							</div>
						</form>
					</div>
					<!---------------------------- end form add ---------------------------------->

					<!---------------------------- form edit sites ------------------------------->
					<div class="add-edit-site" id="div_editsite" style="display: none;">
						<form enctype="multipart/form-data" id="siteform_edit" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="edit_sitename">Site name:</label>
										<input type="text" class="form-control" id="edit_sitename" name="edit_sitename" value="" readonly>
										<input type="hidden" class="form-control" id="edit_siteid" name="edit_siteid" value="">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="shortname">Short name:</label>
										<input type="text" class="form-control" id="shortname" name="shortname" value="">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="seasonEdit"> Season:</label>
										<select id="seasonEdit" class="form-control" name="seasonEdit" onchange="displaySeason('seasonEdit')">
											<option value="0">Winter</option>
											<option value="1">Summer</option>
											<option value="2" selected>Winter and Summer</option>
										</select>
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-2">
									<div id="div_startseason1Edit"
										class="form-group form-group-sm">
										<label class="control-label" for="startseason_winterEdit">Winter from:</label>
										<input type="text" class="form-control" id="startseason_winterEdit" name="startseason_winterEdit" value="">
									</div>
								</div>
								<div class="col-md-2">
									<div id="div_endseason1Edit" class="form-group form-group-sm">
										<label class="control-label" for="endseason_winterEdit">Winter to:</label>
										<input type="text" class="form-control" id="endseason_winterEdit" name="endseason_winterEdit" value="">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-2">
									<div id="div_startseason2Edit"
										class="form-group form-group-sm">
										<label class="control-label" for="startseason_summerEdit">Summer from:</label>
										<input type="text" class="form-control" id="startseason_summerEdit" name="startseason_summerEdit">
									</div>
								</div>
								<div class="col-md-2">
									<div id="div_endseason2Edit" class="form-group form-group-sm">
										<label class="control-label" for="endseason_summerEdit">Summer to:</label>
										<input type="text" class="form-control" id="endseason_summerEdit" name="endseason_summerEdit">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="zip_fileEdit">Upload site shape file:</label>
										<input type="file" class="form-control" id="zip_fileEdit" name="zip_fileEdit">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="edit_enabled">Enable site:</label>
										<input type="checkbox" name="edit_enabled" id="edit_enabled">
									</div>
								</div>
							</div>
							<div class="submit-buttons">
								<input class="add-edit-btn" name="edit_site" type="submit" value="Save Site">
								<input class="add-edit-btn" name="abort_edit" type="button" value="Abort" onclick="abortEditAdd('edit')">
							</div>
						</form>
					</div>
					<!------------------------------ end form edit sites -------------------------------->

					<!------------------------------ list of sites -------------------------------------->
					<table class="table table-striped">
						<thead>
							<tr>
								<th rowspan='2'>Site name</th>
								<th rowspan='2'>Short name</th>
								<th>Winter season</th>
								<th>Summer season</th>
								<th rowspan='2'>Edit</th>
								<th rowspan='2'>Enabled</th>
							</tr>
							<tr>
								<th style="font-size: 11px;">[yyyy-mm-dd]&nbsp;&nbsp;[yyyy-mm-dd]</th>
								<th style="font-size: 11px;">[yyyy-mm-dd]&nbsp;&nbsp;[yyyy-mm-dd]</th>
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
							
							function seasonStartEnd($start, $stop) {
								$currentYear     = date ( 'Y' );
								$currentMonth    = date ( 'm' );
								
								$startSeasonYear = (count($start) == 4) ? (int)($start[2] . $start[3]) : $currentYear;
								$startSeasonMonth = $start[0];
								$startSeasonDay   = $start[1];
								
								$endSeasonYear   = (count($stop)  == 4) ? (int)($stop[2]  . $stop[3])  : $currentYear;
								$endSeasonMonth   = $stop[0];
								$endSeasonDay     = $stop[1];
								
								//check if the season comprises 2 consecutive years (e.q. from october to march next year)
								$startDt = strtotime($startSeasonYear . '-' . $startSeasonMonth . '-' . $startSeasonDay . ' 00:00:00');
								$endDt   = strtotime($endSeasonYear   . '-' . $endSeasonMonth   . '-' . $endSeasonDay   . ' 00:00:00');
								if ((count($start) < 4) || (count($stop) < 4)) {
									// missing year information in database
									if ($startDt > $endDt) {
										if ($currentMonth < $startSeasonMonth) {
											$startSeasonYear = $startSeasonYear - 1;
										} else {
											$endSeasonYear = $startSeasonYear + 1;
										}
									}
								} else {
									// complete year information in database
									if ($startDt > $endDt) {
										// inconsistent season start/stop dates
									}
								}
								
								return array( $startSeasonYear . "-" . $startSeasonMonth . "-" . $startSeasonDay,
											  $endSeasonYear   . "-" . $endSeasonMonth   . "-" . $endSeasonDay );
							}
							
							$sql_select = "";
							$result = "";
							$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

							if ($nr_site == '0') {
								$sql_select = "SELECT * FROM sp_get_dashboard_sites_seasons(null)";
								$result = pg_query_params ( $db, $sql_select, array () ) or die ( "Could not execute." );
							} else {
								$sql_select = "SELECT * FROM sp_get_dashboard_sites_seasons($1)";
								$result = pg_query_params ( $db, $sql_select, array ($_SESSION ['siteId']) ) or die ( "Could not execute." );
							}

							while ( $row = pg_fetch_row ( $result ) ) {
								$siteId       = $row[0];
								$siteName     = $row[1];
								$shortName    = $row[2];
								$summerStart  = $row[3];
								$summerEnd    = $row[4];
								$winterStart  = $row[5];
								$winterEnd    = $row[6];
								$site_enabled = ($row[7] == "t") ? true : false;

								$summer1 = "";
								$summer2 = "";
								
								if ((strlen($summerStart) == 4 || strlen($summerStart) ==  8) &&
									(strlen($summerEnd) == 4 || strlen($summerEnd) == 8)) {
									$summerDate1 = str_split( $summerStart, 2 ) ;
									$summerDate2 = str_split( $summerEnd,   2 ) ;
									$summerStartEnd = seasonStartEnd($summerDate1, $summerDate2);
									$summer1 = $summerStartEnd[0];
									$summer2 = $summerStartEnd[1];
									$summerSeason = $summer1 . "&nbsp;&nbsp;&nbsp;&nbsp;" . $summer2;
								} else {
									$summerSeason = "-";
								}

								$winter1 = "";
								$winter2 = "";
								if ((strlen($winterStart) == 4 || strlen($winterStart) ==  8) &&
									(strlen($winterEnd) == 4 || strlen($winterEnd) == 8)) {
									$winterDate1 =  str_split( $winterStart, 2 ) ;
									$winterDate2 =  str_split( $winterEnd,   2 ) ;
									$winterStartEnd = seasonStartEnd($winterDate1, $winterDate2);
									$winter1 = $winterStartEnd[0];
									$winter2 = $winterStartEnd[1];
									$winterSeason = $winter1 . "&nbsp;&nbsp;&nbsp;&nbsp;" . $winter2;
								} else {
									$winterSeason = "-";
								}

								$tr_table =
										"<tr>"
											. "<td>" . $siteName . "</td>"
											. "<td>" . $shortName . "</td>"
											. "<td>" . $winterSeason . "</td>"
											. "<td>" . $summerSeason . "</td>"
											. "<td class=\"link\"><a onclick=\"formEditSite('" . $siteId . "','" . $siteName . "','" . $shortName . "','" . $winter1 . "','" . $winter2 . "','" . $summer1 . "','" . $summer2 . "'," . ($site_enabled ? "true" : "false" ) . ")\">Edit</a>" . "</td>"
											. "<td><input type=\"checkbox\" name=\"enabled-checkbox\" " . ($site_enabled ? "checked" : "" ) . "></td>"
										. "</tr>";

								$sites = $sites . $tr_table;
								$count ++;
							}
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
					<!------ list sites ------->
				</div>
			</div>
			<!-- End code for adding site---------- -->
		</div>
	</div>
</div>

<!-- includes for datepicker -->
<link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
<script src="libraries/jquery-ui/jquery-ui.min.js"></script>

<!-- includes for bootstrap-switch -->
<link rel="stylesheet" href="libraries/bootstrap-switch/bootstrap-switch.min.css">
<script src="libraries/bootstrap-switch/bootstrap-switch.min.js"></script>

<!-- includes for validate form -->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>

<script type="text/javascript">
$(document).ready( function() {
	// create dialog for add site form
	$("#div_addsite").dialog({
		title: "Add New Site",
		width: '560px',
		autoOpen: false,
		modal: true,
		resizable: false,
		beforeClose: function( event, ui ) { resetEditAdd("add"); }
	});

	// create dialog for edit site form
	$("#div_editsite").dialog({
		title: "Edit Site",
		width: '560px',
		autoOpen: false,
		modal: true,
		resizable: false,
		beforeClose: function( event, ui ) { resetEditAdd("edit"); }
	});

	// change row style when site editing
	$( ".create-site a" ).click(function() {
		$(this).parent().parent().addClass("editing")
	});

	// create switches for all enabled fields in the sites list
	$("[name='enabled-checkbox']").bootstrapSwitch({
		size: "mini",
		onColor: "success",
		offColor: "default",
		disabled: true,
		handleWidth: 25
	});

	// create switch for enabled checkbox in the add site form
	$("[name='add_enabled']").bootstrapSwitch({
		size: "small",
		onColor: "success",
		offColor: "default"
	});

	// create switch for enabled checkbox in the edit site form
	$("[name='edit_enabled']").bootstrapSwitch({
		size: "small",
		onColor: "success",
		offColor: "default"
	});

	// datepickers for add site form
	$("#startseason_winter").datepicker({
		dateFormat: "yy-mm-dd"
	});
	$('#endseason_winter').datepicker({
		dateFormat: "yy-mm-dd"
	});
	$("#startseason_summer").datepicker({
		dateFormat: "yy-mm-dd"
	});
	$('#endseason_summer').datepicker({
		dateFormat: "yy-mm-dd"
	});
	// onChange event for start datepickers for add site form
	$("#startseason_winter").on('change dp.change', function() {
		var date1 = $('#startseason_winter').datepicker('getDate');
		date1.setDate(date1.getDate() + 1);
		if ($('#endseason_winter').datepicker('getDate') === null) {
			$('#endseason_winter').datepicker('setDate', date1);
		}
	});
	$("#startseason_summer").on('change dp.change', function() {
		var date1 = $('#startseason_summer').datepicker('getDate');
		date1.setDate(date1.getDate() + 1);
		if ($('#endseason_summer').datepicker('getDate') === null) {
			$('#endseason_summer').datepicker('setDate', date1);
		}
	});

	// datepickers for edit site form
	$( "#startseason_winterEdit" ).datepicker({
		dateFormat: "yy-mm-dd"
	});
	$( "#endseason_winterEdit" ).datepicker({
		dateFormat: "yy-mm-dd"
	});
	$("#startseason_summerEdit").datepicker({
		dateFormat: "yy-mm-dd"
	});
	$('#endseason_summerEdit').datepicker({
		dateFormat: "yy-mm-dd"
	});
	// onChange event for start datepickers for edit site form
	$("#startseason_winterEdit").on('change dp.change', function() {
		var date1 = $('#startseason_winterEdit').datepicker('getDate');
		date1.setDate(date1.getDate() + 1);
		if ($('#endseason_winterEdit').datepicker('getDate') === null) {
			$('#endseason_winterEdit').datepicker('setDate', date1);
		}
	});
	$("#startseason_summerEdit").on('change dp.change', function() {
		var date1 = $('#startseason_summerEdit').datepicker('getDate');
		date1.setDate(date1.getDate() + 1);
		if ($('#endseason_summerEdit').datepicker('getDate') === null) {
			$('#endseason_summerEdit').datepicker('setDate', date1);
		}
	});

	// validate add site form
	$("#siteform").validate({
		rules: {
			sitename:{ required: true, pattern: "[A-Z]{1}[a-zA-Z_ ]*" },
			startseason_winter : "required",
			endseason_winter: "required",
			startseason_summer: "required",
			endseason_summer: "required",
			zip_fileAdd: "required"
		},
		messages: {
			sitename: { pattern : "First letter must be uppercase.Letters,space and underscore are allowed" }
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
					$("#siteform")[0].reset();
				}
			});
		},
		// set this class to error-labels to indicate valid fields
		success: function(label) {
			label.remove();
		},
	});

	// validate edit site form
	$("#siteform_edit").validate({
		rules: {
			shortname:{ required: true, pattern: "[a-z]{1}[a-z_ ]*" },
			startseason_winterEdit : "required",
			endseason_winterEdit: "required",
			startseason_summerEdit: "required",
			endseason_summerEdit: "required"
		},
		messages: {
			shortname: { pattern : "Only small letters,space and underscore are allowed" }
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

	// display OK/NOK message after the form has been posted
	<?php
	if ( isset($_SESSION['status']) ){
		if ( $_SESSION['status'] =='OK' ) {
			echo "alert('".$_SESSION['message']."')";
			unset($_SESSION['status']);
			unset($_SESSION['message']);
		} else if ( $_SESSION['status']=='NOK' && isset($_SESSION['result']) ){
			echo "alert('FAILED: ".$_SESSION['message']." ".$_SESSION['result'] ."')";
			unset($_SESSION['status']);
			unset($_SESSION['message']);
			unset($_SESSION['result']);
		}
	}
	?>
});

// Open add site form
function formAddSite(){
	// reset all form fields
	resetEditAdd("add");

	// open add site dialog and close all others
	$("#div_editsite").dialog("close");
	$("#div_addsite").dialog("open");
}

// Open edit site form
function formEditSite(id, name, short_name, winter1, winter2, summer1, summer2, site_enabled) {
	// set values for all edited fields
	$("#edit_sitename").val(name);
	$("#edit_siteid").val(id);
	$("#shortname").val(short_name);
	$("#edit_enabled").bootstrapSwitch('state', site_enabled);

	// select season
	if (summer1.length + summer2.length + winter1.length + winter2.length == 40) {
		// Winter and Summer
		$("#seasonEdit").val(2);
	} else if (summer1.length + summer2.length == 20) {
		// Summer
		$("#seasonEdit").val(1);
	} else {
		// Winter
		$("#seasonEdit").val(0);
	}
	displaySeason("seasonEdit");

	$("#startseason_summerEdit").val(summer1);
	$("#endseason_summerEdit").val(summer2);
	$("#startseason_winterEdit").val(winter1);
	$("#endseason_winterEdit").val(winter2);

	// open edit site dialog and close all others
	$("#div_addsite").dialog("close");
	$("#div_editsite").dialog("open");
};

// Reset add/edit site event
function resetEditAdd(formName) {
	if (formName == "add") {
		var validator = $("#siteform").validate();
		validator.resetForm();
		$("#siteform")[0].reset();
	} else if (formName == "edit") {
		var validator = $("#siteform_edit").validate();
		validator.resetForm();
		$("#siteform_edit")[0].reset();
	}
	$( ".create-site tr").removeClass("editing");
}

// Abort add/edit site event
function abortEditAdd(abort){
	if (abort == 'add') {
		$("#div_addsite").dialog("close");
	} else if (abort == 'edit') {
		$("#div_editsite").dialog("close");
	}
}

// Select season event
function displaySeason(param){
	var concat = (param == "seasonEdit" ? "Edit" : "");
	switch ($("#"+param).val()) {
		case "0":
			$("#div_startseason1"+concat).css("display", "block");
			$("#div_endseason1"+concat).css("display", "block");
			$("#div_startseason2"+concat).css("display", "none");
			$("#div_endseason2"+concat).css("display", "none");
			break;
		case "1":
			$("#div_startseason1"+concat).css("display", "none");
			$("#div_endseason1"+concat).css("display", "none");
			$("#div_startseason2"+concat).css("display", "block");
			$("#div_endseason2"+concat).css("display", "block");
			break;
		case "2":
			$("#div_startseason1"+concat).css("display", "block");
			$("#div_endseason1"+concat).css("display", "block");
			$("#div_startseason2"+concat).css("display", "block");
			$("#div_endseason2"+concat).css("display", "block");
			break;
	}
}
</script>

<?php include 'ms_foot.php'; ?>
