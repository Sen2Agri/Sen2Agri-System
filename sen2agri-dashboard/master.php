<?php
session_start();

if (!isset($_SESSION['siteId'])) {
	header("Location: login.php");
}

include 'ms_doc.php';
include 'ms_head.php';
include 'ms_menu.php';
?>