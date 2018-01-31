<?php
session_start();
require_once ("ConfigParams.php");

function getDashboardCurrentJobData($page) {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query_params($dbconn, "select * from sp_get_dashboard_current_job_data($1)", array($page)) or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}
function getDashboardServerResourceData() {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query($dbconn, "select * from sp_get_dashboard_server_resource_data()") or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}
function getDashboardProcessorStatistics() {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query($dbconn, "select * from sp_get_dashboard_processor_statistics()") or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}
function getDashboardProductAvailability($since) {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query_params($dbconn, "select * from sp_get_dashboard_product_availability($1)", array($since)) or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}
function getDashboardJobTimeline($jobId) {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query_params($dbconn, "select * from sp_get_dashboard_job_timeline($1)", array($jobId)) or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}
function getDashboardSites() {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query($dbconn, "select * from sp_get_dashboard_sites()") or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}
function getDashboardProducts($siteId, $processorId) {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query_params($dbconn, "select * from sp_get_dashboard_products($1,$2)", array($siteId, $processorId)) or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}
function getDashboardProcessors() {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query($dbconn, "select * from sp_get_dashboard_processors()") or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
	// GET actions
	$result = "";
	$action = $_GET["action"];
	switch ($action) {
		case "getDashboardCurrentJobData":
			$page = $_GET["page"];
			$result = getDashboardCurrentJobData($page);
			break;
		case "getDashboardServerResourceData":
			$result = getDashboardServerResourceData();
			break;
		case "getDashboardProcessorStatistics":
			$result = getDashboardProcessorStatistics();
			break;
		case "getDashboardProductAvailability":
			$since = $_GET["since"];
			$result = getDashboardProductAvailability($since);
			break;
		case "getDashboardJobTimeline":
			$jobId = $_GET["jobId"];
			$result = getDashboardJobTimeline($jobId);
			break;
		case "getDashboardSites":
			$result = getDashboardSites();
			break;
		case "getDashboardProducts":
			$siteId = $_GET["siteId"];
			$processorId = $_GET["processorId"];
			$result = getDashboardProducts($siteId, $processorId);
			break;
		case "getDashboardProcessors":
			$result = getDashboardProcessors();
			break;
	}
	echo $result;
}

if ($_SERVER['REQUEST_METHOD'] === 'POST'){
	// POST actions
	$result = "";
	$action = $_POST["action"];
	$jobId = $_POST["jobId"];
	
	switch ($action) {
		case "pauseJob":			
			exec('job_operations.py -j '.$jobId.' -o pause', $output, $ret);
			break;
		case "resumeJob":
			exec('job_operations.py -j '.$jobId.' -o resume', $output, $ret);					
			break;
		case "cancelJob":
			exec('job_operations.py -j '.$jobId.' -o cancel', $output, $ret);
			break;	
	}
	
}
?>
