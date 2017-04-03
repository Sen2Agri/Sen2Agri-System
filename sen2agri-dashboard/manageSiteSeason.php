<?php
	session_start();
	require_once('ConfigParams.php');
	
	function saveSiteSeason($id, $site_id, $name, $start, $middle, $end, $enabled) {
		$ret = ""; 
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		if ($id == 0) {
			$result = pg_query_params ($db, "SELECT sp_insert_season($1,$2,$3,$4,$5,$6)",
									array(	$site_id,
											$name,
											$start,
											$end,
											$middle,
											$enabled )
			) or die ("An error occurred.");
			if($result !== FALSE) {
				$ret = "SUCCESS: added " . pg_fetch_result($result, 0, 0);
			} else {
				$ret =  "ERROR: The season was not added.";
			}
		} else {
			$result = pg_query_params ($db, "SELECT sp_update_season($1,$2,$3,$4,$5,$6,$7)",
									array(	$id,
											$site_id,
											$name,
											$start,
											$end,
											$middle,
											$enabled )
			) or die ("An error occurred.");
			$ret = "SUCCESS: edited " . $id;
		}
		return $ret;
	}
	function removeSiteSeason($id) {
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$result = pg_query_params ($db, "SELECT sp_delete_season($1)", array($id)) or die ("An error occurred.");
		return "SUCCESS: removed " . $result;
	}
	
	if (isset($_REQUEST["action"])) {
		$season_id = $_REQUEST["seasonId"];
		if ($_REQUEST["action"] == "save") {
			$site_id        = $_REQUEST["siteId"];
			$season_name    = $_REQUEST["seasonName"];
			$season_start   = $_REQUEST["seasonStart"];
			$season_mid     = $_REQUEST["seasonMiddle"];
			$season_end     = $_REQUEST["seasonEnd"];
			$season_enabled = $_REQUEST["seasonEnabled"];
			$ret = saveSiteSeason($season_id, $site_id, $season_name, $season_start, $season_mid, $season_end, $season_enabled);
		} elseif ($_REQUEST["action"] == "remove") {
			$ret = removeSiteSeason($season_id);
		}
		echo $ret;
	}
?>
