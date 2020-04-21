<?php
	session_start();
	require_once ("ConfigParams.php");
	
	function normalizePath($path) {
		$path = str_replace('\\', '/', $path);
		$path = preg_replace('/\/+/', '/', $path);
		return $path;
	}
	
	// get preview image from database using full_path and quicklook_image
	$dbconn = pg_connect( ConfigParams::getConnection() ) or die ( "Could not connect" );
	$rows = pg_query($dbconn, "select full_path, quicklook_image from product where id=".$_GET['id']) or die(pg_last_error());
	$image_path = false;
	if (pg_numrows($rows) > 0) {
		$absolute_path = pg_fetch_array($rows, 0)[0];
		$quicklook_img = pg_fetch_array($rows, 0)[1];
        // check if the quicklook is an absolute path
        if (realpath($quicklook_img)) {
            $image_path = $quicklook_img;
        } else {
            if (is_file($absolute_path)) {
                $absolute_path = rtrim(normalizePath($absolute_path), '/');
                $absolute_path = dirname($absolute_path);
            }
            $absolute_path = rtrim(normalizePath($absolute_path), '/');
            $image_path = $absolute_path.'/'.$quicklook_img;
        }
	}
	if ($image_path) {
		header('Content-Type: image/jpeg');
		readfile($image_path);
	}
?>
