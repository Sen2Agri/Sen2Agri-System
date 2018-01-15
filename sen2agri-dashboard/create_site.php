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

// processing add site
if (isset ( $_REQUEST ['add_site'] ) && $_REQUEST ['add_site'] == 'Save New Site') {
	// first character to uppercase.
	$site_name = ucfirst ( $_REQUEST ['sitename'] );
	$site_enabled = "0"; // empty($_REQUEST ['add_enabled']) ? "0" : "1";
	
	function insertSite($site, $coord, $enbl) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$sql = "SELECT sp_dashboard_add_site($1,$2,$3)";
		$res = pg_prepare ( $db, "my_query", $sql );
		$res = pg_execute ( $db, "my_query", array (
				$site,
				$coord,
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
		insertSite($site_name, $coord_geog, $site_enabled);
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
	
	function polygonFileSelected($name) {
		foreach($_FILES as $key => $val){
			if (($key == $name) && (strlen($_FILES[$key]['name'])) > 0) {
				return true;
			}
		}
		return false;
	}
	
	function updateSite($id, $enbl) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$res = pg_query_params ( $db, "SELECT sp_dashboard_update_site($1,$2)", array (
				$id,
				$enbl
		) ) or die ( "An error occurred." );
	}
	
	$date = date_create();
	$time_stamp = date_timestamp_get($date);
	
	// upload polygons if zip file selected
	$status     = "OK";
	$message    = "";
	/*
	$shape_file = null;
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
	*/
	if ($status == "OK") {
		updateSite($site_id, $site_enabled);
		$message = "Your site has been successfully modified!";
	}
	$_SESSION['status'] =  $status; $_SESSION['message'] = $message;
	
	// Prevent updating site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}
// processing  delete_site

if (isset ( $_REQUEST ['delete_site'] ) && $_REQUEST ['delete_site'] == 'Delete Site') {
	$site_id      = $_REQUEST ['edit_siteid'];
	$shortname    = $_REQUEST ['shortname'];
//	$site_enabled = empty($_REQUEST ['edit_enabled']) ? "0" : "1";
	
	function deleteSite($id) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$res = pg_query_params ( $db, "SELECT sp_delete_site($1)", array (
				$id
		) ) or die ( "An error occurred.!Delete!" );
	}
	
	$date = date_create();
	$time_stamp = date_timestamp_get($date);
	
	$status     = "OK";
	$message    = "";

	if ($status == "OK") {
		//updateSite($site_id, $site_enabled);
		$message = "Your site has been successfully removed!";
	}
	$_SESSION['status'] =  $status; $_SESSION['message'] = $message;
	
	// Prevent updating site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}

//end - delete site

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
					<?php } else {
						// admin ?>
						<div class="panel-heading row"><input name="addsite" type="button" class="add-edit-btn" value="Create new site" onclick="formAddSite()" style="width: 200px"></div>
					<?php } ?>
					<!---------------------------  form  add site ------------------------>
					<div class="add-edit-site" id="div_addsite" style="display: none;">
						<form enctype="multipart/form-data" id="siteform" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-1">
									<div class="form-group  form-group-sm">
										<label class="control-label" for="sitename">Site name:</label>
										<input type="text" class="form-control" id="sitename" name="sitename">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" style="color:gray">Seasons:</label>
										<h6 style="color:gray;padding:0px 10px 10px 10px;">Seasons can only be added/modified after site creation</h6>
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
							<div class="submit-buttons">
								<input class="add-edit-btn" name="add_site" type="submit" value="Save New Site">
								<input class="add-edit-btn" name="abort_add" type="button" value="Abort" onclick="abortEditAdd('add')">
							</div>
						</form>
					</div>
					<!---------------------------- end form add ---------------------------------->

<!---------------------------  ############################################################################################### ------------------------>
					<!---------------------------  form  delete site ------------------------>
					<div class="add-edit-site" id="div_deletesite" style="display: none;">
						<form enctype="multipart/form-data" id="siteform" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-1">
									<div class="form-group  form-group-sm">
										<label class="control-label" for="edit_sitename">Site name: </label>
										<input type="text" class="form-control" id="edit_sitename" name="edit_sitename" value="" readonly>
										<input type="hidden" class="form-control" id="edit_siteid" name="edit_siteid" value="">
									</div>
								</div>
							</div>
											<div class="form-group form-group-sm sensor">
												<label  style="">To delete:</label>
												<input class="form-control chkL1C" id="delete_chkL1C" type="checkbox" name="sensor" value="L1C" checked="checked">
												<label class="control-label" for="delete_chkL1C">L1C</label>
												<input class="form-control chkL2A" id="delete_chkL2A" type="checkbox" name="sensor" value="L2A" checked="checked">
												<label class="control-label" for="delete_chkL2A">L2A</label>
												<input class="form-control chkL3A" id="delete_chkL3A" type="checkbox" name="sensor" value="L3A" checked="checked">
												<label class="control-label" for="delete_chkL3A">L3A</label>
												<input class="form-control chkL3B" id="delete_chkL3B" type="checkbox" name="sensor" value="L3B" checked="checked">
												<label class="control-label" for="delete_chkL3b">L3B</label>
												<input class="form-control chkL4A" id="delete_chkL4A" type="checkbox" name="sensor" value="L4A" checked="checked">
												<label class="control-label" for="delete_chkL4A">L4A</label>
												<input class="form-control chkL4B" id="delete_chkL4B" type="checkbox" name="sensor" value="L4B" checked="checked">
												<label class="control-label" for="delete_chkL4B">L4B</label>
											</div>
							
							
							
							<div class="submit-buttons">
								<input class="delete-btn" name="delete_site" type="delete" value="Confirm Delete Site">
								<input class="add-edit-btn" name="abort_add" type="button" value="Abort" onclick="abortEditAdd('delete_site')">
							</div>
						</form>
					</div>
					<!---------------------------- end form Delete ---------------------------------->
<!---------------------------  ############################################################################################### ------------------------>					
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
										<label class="control-label">List of Seasons</label>
										<div id="site-seasons"><img src="./images/loader.gif" width="64px" height="64px"></div>
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
							    <!--input class="delete-btn" name="delete_site" type="button" value="Delete Site" onclick="checkDelete("<?= $siteName ?>")"-->
								<input class="delete-btn" name="delete_site" type="button" value="Delete Site" onclick="formDeleteSite()">
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
								<th rowspan="2">Site name</th>
								<th rowspan="2">Short name</th>
								<th class="seasons">Seasons</th>
								<th rowspan="2">Edit</th>
								<th rowspan="2">Enabled</th>
							</tr>
							<tr>
								<th>
									<table class="subtable"><thead><th>Season name</th><th>Season start</th><th>Season mid</th><th>Season end</th><th>Enabled</th></thead><tbody></tbody></table>
								</th>
							</tr>
						</thead>
						<tbody>
						<?php
						$result = "";
						$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
						if (empty($_SESSION['siteId'])) {
							$sql_select = "SELECT * FROM sp_get_sites(null)";
							$result = pg_query_params ( $db, $sql_select, array () ) or die ( "Could not execute." );
						} else {
							$sql_select = "SELECT * FROM sp_get_sites($1)";
							$result = pg_query_params ( $db, $sql_select, array($_SESSION['siteId']) ) or die ( "Could not execute." );
						}
						while ( $row = pg_fetch_row ( $result ) ) {
							$siteId        = $row[0];
							$siteName      = $row[1];
							$shortName     = $row[2];
							$site_enabled  =($row[3] == "t") ? true : false;
							?>
							<tr data-id="<?= $siteId ?>">
								<td><?= $siteName ?></td>
								<td><?= $shortName ?></td>
								<td class="seasons"></td>
								<td class="link"><a onclick='formEditSite(<?= $siteId ?>,"<?= $siteName ?>","<?= $shortName ?>",<?= $site_enabled ? "true" : "false" ?>);'>Edit</a></td>
								<td><input type="checkbox" name="enabled-checkbox"<?= $site_enabled ? "checked" : "" ?>></td>
							</tr>
						<?php } ?>
						</tbody>
					</table>
					<!------------------------------ end list sites ------------------------------>
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
	// get all site seasons
	$("table.table td.seasons").each(function() {
		getSiteSeasons($(this).parent().data("id"));
	});
	
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
		width: '700px',
		autoOpen: false,
		modal: true,
		resizable: false,
		beforeClose: function( event, ui ) { resetEditAdd("edit"); }
	});
	
		// create dialog for delete site form
	$("#div_deletesite").dialog({
		title: "Delete Site",
		width: '700px',
		autoOpen: false,
		modal: true,
		resizable: false,
		beforeClose: function( event, ui ) { resetEditAdd("delete"); }
	});
	
	// change row style when site editing
	$( ".create-site a" ).click(function() {
		$(this).parent().parent().addClass("editing")
	});
	
	// create switches for all checkboxes in the sites list
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
	
	// validate add site form
	$("#siteform").validate({
		rules: {
			sitename:{ required: true, pattern: "[A-Z]{1}[\\w ]*" },
			zip_fileAdd: "required"
		},
		messages: {
			sitename: { pattern : "First letter must be uppercase. Letters, digits, spaces and underscores are allowed" }
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
			shortname:{ required: true, pattern: "[a-z]{1}[a-z_ ]*" }
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

function getSiteSeasons(site_id) {
	var collection = "table.table tr[data-id='" + site_id + "'] td.seasons";
	$(collection).each(function() {
		var season = this;
		$.ajax({
			url: "getSiteSeasons.php",
			type: "get",
			cache: false,
			crosDomain: true,
			data: { "siteId": $(season).parent().data("id"), "action": "get" },
			dataType: "html",
			success: function(data) {
				if (data.length > 0) {
					$(season).html(data);
					$(season).find("input[name='season_enabled']").bootstrapSwitch({
						size: "mini",
						onColor: "success",
						offColor: "default",
						disabled: true,
						handleWidth: 25
					});
				} else {
					$(season).html("-");
				}
			},
			error: function (responseData, textStatus, errorThrown) {
				console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			}
		});	
	});
}

// Open add site form
function formAddSite(){
	// reset all form fields
	resetEditAdd("add");

	// open add site dialog and close all others
	$("#div_editsite").dialog("close");
	$("#div_addsite").dialog("open");
}

// open delete site dialog
function formDeleteSite(name){
	// set values for fields
	$("#delete_sitename").val(name);
	//$("#delete_siteid").val(id);
	//$("#shortname").val(short_name);
	
	// open add site dialog and close all others
//	$("#div_editsite").dialog("close");
	$("#div_addsite").dialog("close");
	$("#div_deletesite").dialog("open");
		$.ajax({
		url: "deletSite.php",
		type: "get",
		cache: false,
		crosDomain: true,
		data:  { "siteId": id, "action": "get" },
		dataType: "html",
		success: function(data) {
			$("#delete_site").html(data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});	

};

/*
function checkDelete(id){

    ids = $(parent).data("id");

	//ids=$(parent).data.val(id);
	conf = confirm("Are you sure you want to delete? id: " + id);
	if (conf)
		
	    $.ajax({
		url: "deleteSite.php",
		type: "get",
		cache: false,
		crosDomain: true,
		data:  { "siteId": id, "action": "get" },
		dataType: "html",
		success: function(data) {
			$("#delete_site").html(data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});	

	else
		return confirm("false");
    
};
*/

// Open edit site form
function formEditSite(id, name, short_name, site_enabled) {
	// set values for all edited fields
	$("#edit_sitename").val(name);
	$("#edit_siteid").val(id);
	$("#shortname").val(short_name);
	$("#edit_enabled").bootstrapSwitch('state', site_enabled);
	
	// open edit site dialog and close all others
	$("#div_addsite").dialog("close");
	$("#div_editsite").data("id", id);
	$("#div_editsite").dialog("open");
	
	$.ajax({
		url: "getSiteSeasons.php",
		type: "get",
		cache: false,
		crosDomain: true,
		data:  { "siteId": id, "action": "edit" },
		dataType: "html",
		success: function(data) {
			$("#site-seasons").html(data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});	
	
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
		
		// refresh seasons for edited/added site
		var site_id = $("#div_editsite").data("id");
		getSiteSeasons(site_id);
		$("#div_editsite").removeData("id");
	}
	else if (formName == "delete") {
		var validator = $("#siteform_delete").validate();
		validator.resetForm();
		$("#siteform_delete")[0].reset();
		
		// refresh seasons for edited/added site
//		var site_id = $("#div_deletesite").data("id");
//		getSiteSeasons(site_id);
//		$("#div_deletesite").removeData("id");
	}
	$( ".create-site tr").removeClass("editing");
}

// Abort add/edit site event
function abortEditAdd(abort){
	if (abort == 'add') {
		$("#div_addsite").dialog("close");
	} else if (abort == 'edit') {
		$("#div_editsite").dialog("close");
	} else if (abort == 'delete') {
		$("#div_deletesite").dialog("close");
	}
}
</script>

<?php include 'ms_foot.php'; ?>
