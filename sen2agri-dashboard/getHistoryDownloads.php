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
		
		$numbers = array(0,0,0,0);
		$percentage = array(0,0,0,0);
		while ( $row = pg_fetch_row ( $result ) ) {
			if ($row[0] == 1) {
				$numbers[0] = $row[1];
				$percentage[0] = $row[2];
			} else if($row[0] == 2){
			    $numbers[1] = $row[1];
			    $percentage[1] = $row[2];
			} else if($row[0] == 3){
			    $numbers[2] = $row[1];
			    $percentage[2] = $row[2];
			}
			else if($row[0] == 4){
			    $numbers[3] = $row[1];
			    $percentage[3] = $row[2];
			}
		}
		 echo json_encode(array('numbers'=>$numbers,'percentage'=>$percentage));
	}

?>
