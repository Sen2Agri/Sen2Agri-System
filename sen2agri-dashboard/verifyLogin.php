<?php
session_start();
require_once('ConfigParams.php');

function SignIn()
{
        $dbconn = pg_connect( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

        //starting the session for user
        if(!empty($_POST['user'])) {
                // Interrogate database for credentials
                $result = pg_query_params($dbconn, "SELECT * FROM sp_authenticate($1, $2)", array($_POST['user'], $_POST['pass'])) or die(pg_last_error());
                if ($row = pg_fetch_row($result)) {
                        $userId = $row[0];
                        $siteId = $row[1];
                        $siteId = isset($siteId) ? $siteId : "";

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
