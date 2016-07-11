<?php
	require_once("ConfigParams.php");
	
	if (isset($_REQUEST['siteID_selected'])) {
		$page_no = isset($_REQUEST['page']) ? $_REQUEST['page'] : 1;
		
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$result = "";
		if ($_REQUEST['siteID_selected'] == 0) {
			$result = pg_query_params ( $db, "SELECT * FROM sp_get_job_history($1, $2)", array (null, $page_no) ) or die ( "Could not execute." );
		} else {
			$result = pg_query_params ( $db, "SELECT * FROM sp_get_job_history($1, $2)", array ($_REQUEST['siteID_selected'], $page_no)) or die ( "Could not execute." );
		}
		
		$tr_current ="";
		while ( $row = pg_fetch_row ( $result ) ) {
			$tr = "<tr>".
					"<td>" . $row[0] . "</td>".
					"<td>" . $row[1] . "</td>".
					"<td>" . $row[2] . "</td>".
					"<td>" . $row[3] . "</td>".
					"<td>" . $row[4] . "</td>".
					"<td>" . $row[5] . "</td>".
				"</tr>";
			$tr_current = $tr_current . $tr;
		}
		echo $tr_current;
	}
?>