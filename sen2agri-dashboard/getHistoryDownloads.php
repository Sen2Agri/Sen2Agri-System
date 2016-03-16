<?php
	require_once("ConfigParams.php");


	if(isset($_REQUEST['siteID_selected'])){
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$result_downloads ="";
		if ($_REQUEST['siteID_selected'] == 0){
			$result_downloads = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history(null)", array () ) or die ( "Could not execute." );
		}else {
	$result_downloads = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history($1)", array ($_REQUEST['siteID_selected']) ) or die ( "Could not execute." );
		}
	$td_statistics = "";
	while ( $row = pg_fetch_row ( $result_downloads ) ) {
		$td = "<td colspan=\"2\">" . $row [0] . "</td>";
		$td_statistics = $td_statistics . $td;
	}
	echo $td_statistics;
	}
	//var_dump($_REQUEST['siteID_selected'] == 0);
?>
