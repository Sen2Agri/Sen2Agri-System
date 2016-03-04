<?php
session_start();

if (isset($_SESSION['siteId'])) {
	require_once('ConfigParams.php');
}

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
						<form method="POST" action="verifyLogin.php">
							<label>Username:</label><input type="text" name="user" size="40">
							<label>Password:</label><input type="password" name="pass" size="40">
							</br>
							<input id="button" type="submit" name="submit" value="login">
						</form>
					</fieldset>
				</div>
				<div class="error">
<?php
if (isset($_SESSION['loginMessage'])) {
	echo $_SESSION['loginMessage'];
}
?>
				</div>
				<div class="vspace"></div>
			</div>
        </div>
    </div>
<?php include "ms_foot.php" ?>
