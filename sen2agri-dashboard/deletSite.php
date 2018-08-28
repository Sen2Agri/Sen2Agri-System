<?php
	session_start();
	require_once('ConfigParams.php');
	
	function removeSite($id) {
		echo "here 1";
		$db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
		$result = pg_query_params ($db, "SELECT sp_delete_site($1)", array($id)) or die ("An error occurred.");
		return "SUCCESS: removed " . $result;
	}
	
	
if (isset ( $_REQUEST ['delete_site'] ) && $_REQUEST ['delete_site'] == 'Delete Site') {
	$site_id      = $_REQUEST ['edit_siteid'];
	$shortname    = $_REQUEST ['shortname'];
	$site_enabled = empty($_REQUEST ['edit_enabled']) ? "0" : "1";
	
	$date = date_create();
	$time_stamp = date_timestamp_get($date);
	
	// upload polygons if zip file selected
	$status     = "OK";
	$message    = "";

	if ($status == "OK") {
		removeSite($site_id);
		$message = "Your site has been successfully modified!";
	}
	$_SESSION['status'] =  $status; $_SESSION['message'] = $message;
	
	// Prevent updating site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}

?>
