<?php
session_start();
require_once('ConfigParams.php');

function SignIn()
{
	$dbconn = pg_connect( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	
	//starting the session for user
	if(!empty($_POST['user'])) {
		// Interrogate database for credentials
		$rows = pg_query_params($dbconn, "SELECT sp_authenticate($1, $2)", array($_POST['user'], $_POST['pass'])) or die(pg_last_error());
		if (pg_numrows($rows) > 0) {
			// extract value from first returned row and capure text between brackets
			$result = pg_fetch_array($rows, 0)[0];
			preg_match('#\((.*?)\)#', $result, $match);
			
			// extract userId and siteId from extracted value
			list($userId, $siteId) = split("[,]", $match[1]);
			
			// set session variables
			$_SESSION['userId'] = $userId;
			$_SESSION['siteId'] = $siteId;
			$_SESSION['userName'] = $_POST['user'];
			$_SESSION['loginMessage'] = "";
			header("Location: create_site.php");
			exit;
		} else {
			$_SESSION['loginMessage'] = "Invalid username or password. Please retry!";
		}
	} else {
		$_SESSION['loginMessage'] = "Please enter your credentials!";
	}
	header("Location: login.php");
	exit;
}
if(isset($_POST['submit'])) {
	SignIn();
}
?>
