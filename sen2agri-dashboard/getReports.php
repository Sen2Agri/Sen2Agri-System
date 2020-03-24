<?php
require_once ("ConfigParams.php");

if (isset($_REQUEST['report_type']) && $_REQUEST['report_type'] == "orbit") {
	// ORBIT statistics
	
	$satellite = "";
	$siteId = "";
	$orbit = "";
	$fromDate = "";
	$toDate = "";

	if (isset($_REQUEST['satellite']) && $_REQUEST['satellite'] != "") {
		$satellite = $_REQUEST['satellite'];
	}
	if (isset($_REQUEST['siteId']) && $_REQUEST['siteId'] != "0" && $_REQUEST['siteId'] != "") {
		$siteId = "&siteId=" . $_REQUEST['siteId'];
	}
	if (isset($_REQUEST['orbit']) && $_REQUEST['orbit'] != "0" && $_REQUEST['orbit'] != "") {
		$orbit = "&orbit=" . $_REQUEST['orbit'];
	}
	if (isset($_REQUEST['fromDate']) && $_REQUEST['fromDate'] != "") {
		$fromDate = "&fromDate=" . $_REQUEST['fromDate'];
	}
	if (isset($_REQUEST['toDate']) && $_REQUEST['toDate'] != "") {
		$toDate = "&toDate=" . $_REQUEST['toDate'];
	}
	if (isset($_REQUEST['getOrbitList']) && $_REQUEST['getOrbitList'] != "") {
		$curl = curl_init();
		$url =  ConfigParams::$REST_SERVICES_URL . "/reports/orbit/list?satellite=" . $satellite . $siteId;
		curl_setopt($curl, CURLOPT_URL,  $url );
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
		
		$result = curl_exec($curl);
		echo $result;
		
		curl_close($curl);
	} else {
		$curl = curl_init();
		$url =  ConfigParams::$REST_SERVICES_URL . "/reports/orbit?satellite=" . $satellite . $siteId. $orbit . $fromDate . $toDate;
		curl_setopt($curl, CURLOPT_URL,  $url );
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
		
		$result = curl_exec($curl);
		echo $result;
		
		curl_close($curl);
	}
} else if (isset($_REQUEST['report_type']) && $_REQUEST['report_type'] == "aggregate") {
	// AGGREGATE statistics
	
	$satellite = "";
	$siteId = "";
	$orbit = "";
	$fromDate = "";
	$toDate = "";

	if (isset($_REQUEST['satellite']) && $_REQUEST['satellite'] != "") {
		$satellite = $_REQUEST['satellite'];
	}
	if (isset($_REQUEST['siteId']) && $_REQUEST['siteId'] != "0" && $_REQUEST['siteId'] != "") {
		$siteId = "&siteId=" . $_REQUEST['siteId'];
	}
	if (isset($_REQUEST['orbit']) && $_REQUEST['orbit'] != "0" && $_REQUEST['orbit'] != "") {
		$orbit = "&orbit=" . $_REQUEST['orbit'];
	}
	if (isset($_REQUEST['fromDate']) && $_REQUEST['fromDate'] != "") {
		$fromDate = "&fromDate=" . $_REQUEST['fromDate'];
	}
	if (isset($_REQUEST['toDate']) && $_REQUEST['toDate'] != "") {
		$toDate = "&toDate=" . $_REQUEST['toDate'];
	}
	$curl = curl_init();
	$url =  ConfigParams::$REST_SERVICES_URL . "/reports/l2/aggregate?satellite=" . $satellite . $siteId. $orbit . $fromDate . $toDate;
	curl_setopt($curl, CURLOPT_URL,  $url );
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
	
	$result = curl_exec($curl);
	echo $result;
	
	curl_close($curl);
} else {
	$object = (object) [
		'data' => null,
		'message' => 'Select report type',
		'status' => 'FAILED'
	];
	
	echo json_encode($object);
}

?>