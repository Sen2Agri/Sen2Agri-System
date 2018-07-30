<?php
session_start();
require_once('ConfigParams.php');

function SignIn()
{
    $dbconn = pg_connect( ConfigParams::getConnection() ) or die ( "Could not connect" );

        //starting the session for user
        if(!empty($_POST['user'])) {
                // Interrogate database for credentials
                $result = pg_query_params($dbconn, "SELECT * FROM sp_authenticate($1, $2)", array($_POST['user'], $_POST['pass'])) or die(pg_last_error());
                if ($row = pg_fetch_row($result)) {
                        $userId = $row[0];
                        $siteId = isset($row[1]) ? explode(',', str_replace(['{','}'], ['',''], $row[1])) : array();

                        // set session variables
                        $_SESSION['userId'] = $userId;
                        $_SESSION['siteId'] = $siteId;
                        $_SESSION['userName'] = $_POST['user'];
                        $_SESSION['loginMessage'] = "";
                        $_SESSION['roleID'] = $row[2];
                        $_SESSION['isAdmin'] = $row[2]=='1'?true:false;
                        $_SESSION['roleName'] = $row[3];
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

function setUserPassword(){
    $dbconn = pg_connect( ConfigParams::getConnection() ) or die ( "Could not connect" );
    
    if(!empty($_POST['username']) && !empty($_POST['email'])  && !empty($_POST['password'])  && !empty($_POST['password_confirm'])) {
        $result = pg_query_params($dbconn, "SELECT * FROM sp_set_user_password($1, $2, $3)", array($_POST['username'], $_POST['email'], $_POST['password'] )) or die(pg_last_error());
        $row = pg_fetch_row($result);
        switch ($row['0']){
            case 0:   
                $_SESSION['saveMessage'] = "Invalid password. Please try again!";          
                break;
            case 1:
                $_SESSION['success'] = "Your password was successfully saved.";
                break;
            case 2:
                $_SESSION['saveMessage'] = "User not found. Please try again!";
                break;
        }

        header("Location: login.php");
        exit;
    }else{
        $_SESSION['saveMessage'] = "Invalid entries!";
        header("Location: login.php");
        exit;
    }
}

if(isset($_POST['submit'])) {
    switch ($_POST['submit']){
        case 'save':setUserPassword();break;
        case 'login':
        case 'submit': SignIn();
        break;
    }
	
}
?>
