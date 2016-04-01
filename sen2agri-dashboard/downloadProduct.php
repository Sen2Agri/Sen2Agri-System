<?php
	// use session_start to prevent multiple downloads at a time
	session_start();
	
	$absolute_path = rtrim($_GET['path'], '/'); 									// get absolute path without trailing slash
	$relative_path = substr($absolute_path, 0, strripos($absolute_path, '/'));		// get relative path to folder
	$folder_name = end( explode( "/", $absolute_path ) );							// get folder name
	$gzip_name = $folder_name.".tar.gz";
	
	header('Pragma: no-cache'); 
	header('Content-Description: File Download');
	header('Content-Type: application/x-gzip');
	header("Expires: 0");
	header('Content-Disposition: attachment; filename="'.$gzip_name.'"');
	
	// use popen to execute a unix command pipeline and grab the stdout as a php stream
	// (you can use proc_open instead if you need to control the input of the pipeline too)
	$fp = popen('cd '.$relative_path.' && tar cf - '.$folder_name.' | gzip -c', 'r');
	
	// pick a bufsize that makes you happy (64k may be a bit too big).
	$bufsize = 65535;
	$buff = '';
	while(!feof($fp)) {
		$buff = fread($fp, $bufsize);
		echo $buff;
		ob_flush();
		flush();
	}
	pclose($fp);
?>
