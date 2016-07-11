<?php
	require_once("ConfigParams.php");
	
	if (isset($_REQUEST['siteID_selected'])) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$result = "";
		if ($_REQUEST['siteID_selected'] == 0) {
			$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history(null)", array () ) or die ( "Could not execute." );
		} else {
			$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history($1)", array ($_REQUEST['siteID_selected']) ) or die ( "Could not execute." );
		}
		
		$td1="<td>0</td>";
		$td2="<td>0</td>";
		$td3="<td>0</td>";
		while ( $row = pg_fetch_row ( $result ) ) {
			if ($row[0] == 1) {
				$td1 = "<td>" . $row [1] . "</td>";
			} else if($row[0] == 2){
				$td2 = "<td>" . $row [1] . "</td>";
			} else if($row[0] == 3){
				$td3 = "<td>" . $row [1] . "</td>";
			}
		}
		 echo  $td1.$td2.$td3;
	}

?>
