<?php
session_start();
require_once('ConfigParams.php');

function get_tiles(){
	$tiles = array();
	if(isset($_POST['term'])){
		$db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
		
		$query ="";
		if( sizeof($_POST['satellite_id'])== 2){
			$query = "SELECT * FROM sp_get_site_tiles('".$_POST['site_id']."','1') WHERE tile_id ilike '%".$_POST['term']."%'";
			$query .= " UNION ";
			$query .= "SELECT * FROM sp_get_site_tiles('".$_POST['site_id']."','2') WHERE tile_id ilike '%".$_POST['term']."%'";
		}else{
			$query = "SELECT * FROM sp_get_site_tiles('".$_POST['site_id']."','".$_POST['satellite_id'][0]."') WHERE tile_id ilike '%".$_POST['term']."%'";
		}

		$result = pg_query($db, $query) or die ("Could not execute.");
			   
		if (pg_num_rows($result) > 0) {
			while ( $row = pg_fetch_object( $result ) ) {
				$tiles[]= $row->tile_id;
			}
		}
		echo json_encode($tiles);
	}
	
}

function get_site_seasons(){
	$db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	
	$season = array();
	
	if(isset($_POST['site_id'])){
		// get all seasons for site_id
		$sql = "SELECT id, name FROM sp_get_site_seasons('".$_POST['site_id']."') ORDER BY name";
		$result = pg_query ( $db, $sql ) or die ( "Could not execute." );
		
		while ( $row = pg_fetch_row ( $result ) ) {
			$season[]=$row;
		}
	}
	
	echo json_encode($season);
	
}


if(isset($_POST['action'])){
	switch ($_POST['action']){
		case 'get_tiles': get_tiles();
			break;
		case 'get_site_seasons': get_site_seasons();
			break;
	}
}
?>