<?php
session_start();
include 'ms_doc.php';
include 'ms_head.php';
include 'ms_menu.php';
?>
    <div id="main">
        <div id="main2">
            <div id="main3">
				<div class="vspace"></div>
				<div id="signin">
					<fieldset>
						<form method="POST" action="login.php">
							<label>Username:</label><input type="text" name="user" size="40">
							<label>Password:</label><input type="password" name="pass" size="40">
							</br>
							<input id="button" type="submit" name="submit" value="login">
						</form>
					</fieldset>
				</div>
				<div class="error">
<?php

/*
$UserName = $_POST['user'];
$Password = $_POST['pass'];
*/
function SignIn()
{
	$dbhost = 'sen2agri-dev';
	$dbname = 'sen2agri';
	$dbuser = 'admin';
	$dbpass = 'sen2agri';
	$dbconn = pg_connect("host=".$dbhost." port=5432 dbname=".$dbname." user=".$dbuser." password=".$dbpass)
		or die("Could not connect");
	
	//starting the session for user
	if(!empty($_POST['user'])) {
		// Interrogate database for credentials
		$rows = pg_query_params($dbconn, "SELECT sp_authenticate($1, $2)", array($_POST['user'], $_POST['pass']))
			or die(pg_last_error());
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
			
			echo "SUCCESSFULLY LOGIN...</br></br>";
			
			echo "Username: ".$_POST['user']."</br>";
			echo "Password: ".$_POST['pass']."</br>";
			echo "Returned value: ".$result."</br>";
			echo "UserId: ".$userId."</br>";
			echo "SiteId: ".$siteId."</br>";
			
			header("Location: main.php");
			exit;
			
		} else {
			echo "Invalid username or password... Please retry... ";
		}
	} else {
		echo "Please enter your credentials!";
	}
}
if(isset($_POST['submit'])) {
	SignIn();
}
?>
				</div>
				<div class="vspace"></div>
			</div>
        </div>
    </div><!-- main --><!-- main2 --><!-- main3 -->
	
    <script src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-2.1.4.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.6/d3.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.time.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.stack.min.js"></script>
    <script src="libraries/nvd3-1.1.11/nv.d3.js"></script>
    <script src="libraries/bootstrap-treeview/bootstrap-treeview.min.js"></script>
    <script src="libraries/openlayers/build/ol.js"></script>
    <script src="scripts/config.js"></script>
    <script src="scripts/helpers.js"></script>

<?php include 'ms_foot.php'; ?>
