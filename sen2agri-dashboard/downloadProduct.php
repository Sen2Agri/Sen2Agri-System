<?php
	// use session_start to prevent multiple downloads at a time
	session_start();
	require_once ("ConfigParams.php");
	
	$dbconn = pg_connect( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$rows = pg_query($dbconn, "select full_path from product where id=".$_GET['id']) or die(pg_last_error());
	if (pg_numrows($rows) > 0) {
		$result = pg_fetch_array($rows, 0)[0];
		
		$absolute_path = rtrim($result, '/'); 												// get absolute path without trailing slash
		if (file_exists($absolute_path)) {
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
		} else {
			ob_end_clean();
			$_SESSION['errorMessage'] = "Download failed!</br></br>The product folder is missing.";
			header("Location: errorMessage.php");
			
			exit;
		}
	} else {
		$_SESSION['errorMessage'] = "Download failed!</br></br>The product folder is missing.";
		header("Location: errorMessage.php");
		
		exit;
	}
?>
