<?php
session_start();
require_once('ConfigParams.php');

function getAllUsers(){
	$db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");

    $sql ="SELECT * FROM sp_get_all_users()";
	$result = pg_query ( $db, $sql ) or die ( "Could not execute." );

	if (pg_num_rows($result) > 0) {
	    while ( $row = pg_fetch_assoc( $result) ) {
	        $users[]= $row;
	    }
	}
	
	echo json_encode(array('data'=>$users));
}

function checkUser(){
    $db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
    
    $sql = "SELECT login FROM \"user\" u WHERE u.login ='".$_REQUEST['login']."'";
    if(isset($_REQUEST['id']) && trim($_REQUEST['id'])){
        $sql.=" AND id<>'".$_REQUEST['id']."'";
    }
    $result = pg_query ( $db, $sql ) or die ( "Could not execute." );

    if(pg_num_rows($result) > 0){
        echo 'false';
    }else{
        echo 'true';
    }
}

function insertUser(){
    $db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");

    $sql = "UPDATE \"user\" SET  VALUES($1,$2,$3)";
    $res = pg_prepare ( $db, "my_query", $sql );
    $res = pg_execute ( $db, "my_query", array (
        $login,
        $email,
        $role
    ) ) or die ( "An error occurred." );
    $row = pg_fetch_row($res);
    return $row[0];
}

function updateUser(){
    $db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
    $sql = "SELECT sp_add_update_user($1,$2,$3,$4,$5,$6)";
   /* $sql = "INSERT INTO \"user\" VALUES($1,$2,$3)";
    $res = pg_prepare ( $db, "my_query", $sql );*/
    $res = pg_execute ( $db, "my_query", array (
        $id,
        $login,
        $email,
        $role,
        $site,
        $password
    ) ) or die ( "An error occurred." );
    $row = pg_fetch_row($res);
    return $row[0];
}

function getSites(){
    $db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
    $rows = pg_query($db, "SELECT id, name as text FROM sp_get_sites()") or die(pg_last_error());
    $result = pg_fetch_all($rows);
    echo json_encode(array('results'=>$result));
}

function deleteUser(){
    $db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
    $rows = pg_query_params($db, "DELETE FROM \"user\" WHERE id = $1",array($_POST['user_id'])) or die(pg_last_error());
    
    echo  "The user has been successfully removed!";
}

function addUser(){
    $db = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
    $sites_id = isset($_REQUEST['site'])? '{'.implode(',',$_REQUEST['site']).'}':null;
    $user_id = isset($_REQUEST['user_id'])?$_REQUEST['user_id']:null;
        
   // $sql = "SELECT sp_adduser_1($1,$2,$3,$4)";
    $sql = "SELECT sp_insert_update_user($1,$2,$3,$4,$5)";
    $res = pg_prepare ( $db, "my_query", $sql );
    $res = pg_execute ( $db, "my_query", array (
        $_REQUEST['login'],
        $_REQUEST['email'],
        $user_id,
        $_REQUEST['role'],
        $sites_id
    ) ) or die ( "An error occurred." );
    $row = pg_fetch_row($res);

   // return $row[0];
    echo  "The user '".$_REQUEST['login']."' has been successfully $!";
    
}

if(isset($_REQUEST['action'])){
    switch ($_REQUEST['action']){
	    case 'getAllUsers': getAllUsers();break;
	    case 'checkUser'  : checkUser();break;
	    case 'updateUser' : updateUser();break;
	    case 'getSites'   : getSites();break;
	    case 'delete'     : deleteUser();break;
	
	}
}else if($_SERVER['REQUEST_METHOD'] == 'POST'){
    addUser();
}
?>