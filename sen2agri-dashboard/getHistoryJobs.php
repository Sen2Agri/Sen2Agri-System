<?php
	require_once("ConfigParams.php");

	if (isset($_REQUEST['siteID_selected'])) {
		$page_no = isset($_REQUEST['page']) ? $_REQUEST['page'] : 1;
		$rows_per_page = isset($_REQUEST['rows_per_page']) ? $_REQUEST['rows_per_page'] : 20;

		$db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
		$result = "";
		$result_cnt="";
		if ($_REQUEST['siteID_selected'] == 0) {
			$result_cnt = pg_query_params ( $db, "SELECT count(id) FROM job", array () ) or die ( "Could not execute." );
			$result     = pg_query_params ( $db, "SELECT * FROM sp_get_job_history_custom_page($1, $2, $3)", array (null, $page_no, $rows_per_page) ) or die ( "Could not execute." );
		} else {
			$result_cnt = pg_query_params ( $db, "SELECT count(id) FROM job WHERE site_id=$1", array ($_REQUEST['siteID_selected']) ) or die ( "Could not execute." );
			$result     = pg_query_params ( $db, "SELECT * FROM sp_get_job_history_custom_page($1, $2, $3)", array ($_REQUEST['siteID_selected'], $page_no, $rows_per_page)) or die ( "Could not execute." );
		}

		$row_cnt = pg_fetch_row($result_cnt);
		$tr_current  = "<!--" . $row_cnt[0] . "-->";

		while ( $row = pg_fetch_row ( $result ) ) {
			$tr = "<tr>".
					"<td>" . $row[0] . "</td>".
					"<td>" . (($row[1] == "") ? "-" : $row[1]) . "</td>".
					"<td>" . $row[2] . "</td>".
					"<td>" . $row[3] . "</td>".
					"<td>" . $row[4] . "</td>".
					"<td>" . $row[5] . "</td>".
					'<td><a href="getJobOutput.php?jobId=' . $row[0] . '">[output]</a></td>' .
				"</tr>";
			$tr_current = $tr_current . $tr;
		}
		echo $tr_current;
	}
?>
