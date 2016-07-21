<?php
	require_once("ConfigParams.php");
	
	if (isset($_REQUEST['siteID_selected'])) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$result = "";
		if ($_REQUEST['siteID_selected'] == 0) {
			$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_current_downloads(null)", array () ) or die ( "Could not execute." );
		} else {
			$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_current_downloads($1)", array ($_REQUEST['siteID_selected'])) or die ( "Could not execute." );
		}
		
		$tr_current ="";
		while ( $row = pg_fetch_row ( $result ) ) {
			$tr = "<tr>".
					"<td>" . $row[0] . "</td>".
					"<td>" . $row[1] . "</td>".
					"<td>" . $row[2] . "</td>".
					"<td>" . $row[3] . "</td>".
				"</tr>";
			$tr_current = $tr_current . $tr;
		}
		echo $tr_current;
	}
?>