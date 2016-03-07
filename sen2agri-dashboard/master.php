<?php
session_start();
if (isset($_SESSION['siteId'])) {
	require_once("ConfigParams.php");
} else {
	header("Location: login.php");
}

include "ms_doc.php";
include "ms_head.php";
include "ms_menu.php";
?>
