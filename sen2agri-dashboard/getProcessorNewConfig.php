<?php
session_start();
require_once ("ConfigParams.php");

function redirect_page($processor_short_name, $status, $message) {
	$_SESSION['processor'] = $processor_short_name;
	$_SESSION['status']    = $status;
	$_SESSION['message']   = $message;
	
	// redirect to custom jobs page
	$referer = $_SERVER['HTTP_REFERER'];
	header("Location: $referer");
	exit();
}

function endsWith( $str, $sub ) {
	return ( substr( $str, strlen( $str ) - strlen( $sub ) ) === $sub );
}

function createCustomUploadFolder($site_id, $timestamp) {
	// create custom upload path like: /mnt/upload/siteName/userName_timeStamp/
	$dbconn = pg_connect( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$rows = pg_query($dbconn, "SELECT key, value FROM sp_get_parameters('site.upload_path') WHERE site_id IS NULL") or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[1];
	$upload_target_dir = str_replace("{user}", "", $result);
	$rows = pg_query($dbconn, "SELECT name FROM sp_get_sites() WHERE id = ".$site_id) or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[0];
	$upload_target_dir = $upload_target_dir . $result . "/" . ConfigParams::$USER_NAME . "_".$timestamp . "/";
	$upload_target_dir = str_replace("(", "", $upload_target_dir);
	$upload_target_dir = str_replace(")", "", $upload_target_dir);
	$upload_target_dir = str_replace(" ", "_", $upload_target_dir);
	if (!is_dir($upload_target_dir)) {
		mkdir($upload_target_dir, 0755, true);
	}
	return $upload_target_dir;
}

function upload_reference_polygons($site_id, $timestamp) {
	$zip_msg = '';
	$shp_file = false;
	if($_FILES["refp"]["name"]) {
		$filename = $_FILES["refp"]["name"];
		$source = $_FILES["refp"]["tmp_name"];
		
        $upload_target_dir = createCustomUploadFolder($site_id, $timestamp);
		
        $target_path = $upload_target_dir . $filename;
        if(move_uploaded_file($source, $target_path)) {
            $zip = new ZipArchive();
            $x = $zip->open($target_path);
            if ($x === true) {
                for ($i = 0; $i < $zip->numFiles; $i++) {
                    $filename = $zip->getNameIndex($i);
                    if (endsWith($filename, '.shp')) {
                        $shp_file = $upload_target_dir . $filename;
                        break;
                    }
                }
                $zip->extractTo($upload_target_dir);
                $zip->close();
                unlink($target_path);
                if ($shp_file) {
                    $zip_msg = "Your .zip file was uploaded and unpacked successfully";
                } else {
                    $zip_msg = "Your .zip file does not contain any shape (.shp) file";
                }
            } else {
                $zip_msg = "Your file is not a valid .zip archive";
            }
        } else {
            $zip_msg = "Failed to upload the file you selected";
        }
	} else {
		$zip_msg = 'Unable to access your selected file';
	}
	
	// verify if shape file has valid geometry
	$shp_msg = '';
	$shape_ok = false;
	if ($shp_file) {
		exec('scripts/check_shp.py '.$shp_file, $output, $ret);
		
		if ($ret === FALSE) {
			$shp_msg = 'Invalid command line';
		} else {
			switch ($ret) {
				case 0:		$shape_ok = true; break;
				case 1:		$shp_file = false; $shp_msg = 'Unable to open the shape file'; break;
				case 2:		$shp_file = false; $shp_msg = 'Shape file has invalid geometry'; break;
				case 3:		$shp_file = false; $shp_msg = 'Shape file has overlapping polygons'; break;
				case 127:	$shp_file = false; $shp_msg = 'Invalid geometry detection script'; break;
				default:	$shp_file = false; $shp_msg = 'Unexpected error with the geometry detection script'; break;
			}
		}
		if ($shape_ok) {
            $last_line = $output[count($output) - 1];
            $shp_msg = '';
			/* $r = preg_match('/^Union: (.+)$/m', $last_line, $matches); */
			/* if (!$r) { */
			/* 	$shp_file = false; */
			/* 	$shp_msg = 'Unable to parse shape'; */
			/* } else { */
			/* 	$shp_msg = $matches[1]; */
			/* } */
		}
	} else {
		$shp_msg = 'Missing shape file due to a problem with your selected file';
	}
	
	return array ( "polygons_file" => $shp_file, "result" => $shp_msg, "message" => $zip_msg );
}

function upload_reference_raster($site_id, $timestamp) {
	$shp_file = FALSE;
	if($_FILES["refr"]["name"]) {
		$filename = $_FILES["refr"]["name"];
		$source = $_FILES["refr"]["tmp_name"];
		
        $upload_target_dir = createCustomUploadFolder($site_id, $timestamp);
		
        $target_path = $upload_target_dir . $filename;  // change this to the correct site path
        if(move_uploaded_file($source, $target_path)) {
            $shp_file = $target_path;
        } else {
            $message = "There was a problem with the upload. Please try again.";
            $shp_file = false;
        }
	}
	return $shp_file;
}

function insertjob($name, $description, $processor_short_name, $site_id, $start_type_id, $parameters, $configuration) {
	$db = pg_connect( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	
	$rows = pg_query($db, "SELECT id FROM processor WHERE short_name='$processor_short_name'") or die(pg_last_error());
	if (pg_numrows($rows) > 0) {
		$processor_id = pg_fetch_array($rows, 0)[0];
		
		$sql1 = "SELECT sp_submit_job($1,$2,$3,$4,$5,$6,$7)";
		$res = pg_prepare ( $db, "my_query", $sql1 );
		$res = pg_execute ( $db, "my_query", array (
				$name,
				$description,
				$processor_id,
				$site_id,
				$start_type_id,
				$parameters,
				$configuration
		) ) or die ("An error occurred.");
		
		// send notification through CURL
		try {
			//url of the service
			$url= ConfigParams::$SERVICES_NOTIFY_ORCHESTRATOR_URL;
			
			//initialise connection
			$ch = curl_init();
			curl_setopt($ch, CURLOPT_URL, $url);
			curl_setopt($ch, CURLOPT_POST, 1);
			curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
			
			$response = curl_exec($ch);
			curl_close($ch);
		} catch (Exception $e) {
		}
	} else {
		echo "Invalid processor (". $processor_short_name ."): processor short name not found in the database.";
	}
}

if (isset ( $_POST ['l3a'] )) {
	$processor_short_name = "l3a";
	
	// default parameters
	$siteId           = $_POST ['siteId'];
	$input_products   = $_POST ['inputFiles'];
	$synthDate        = $_POST ['synthDate'];
	$halfSynthesis    = $_POST ['halfSynthesis'];
	$resolution       = $_POST ['resolution'];
	
	// advanced parameters
	$maxaot           = $_POST ['maxaot'];
	$minweight        = $_POST ['minweight'];
	$maxweight        = $_POST ['maxweight'];
	$sigmasmall       = $_POST ['sigmasmall'];
	$sigmalarge       = $_POST ['sigmalarge'];
	$coarseresolution = $_POST ['coarseresolution'];
	$weightdatemin    = $_POST ['weightdatemin'];
	
	$config = array (
		array ( "key"   => "processor.l3a.weight.aot.maxaot",
				"value" => $maxaot ),
		array ( "key"   => "processor.l3a.weight.aot.minweight",
				"value" => $minweight ),
		array ( "key"   => "processor.l3a.weight.aot.maxweight",
				"value" => $maxweight ),
		array ( "key"   => "processor.l3a.weight.cloud.sigmasmall",
				"value" => $sigmasmall ),
		array ( "key"   => "processor.l3a.weight.cloud.sigmalarge",
				"value" => $sigmalarge ),
		array ( "key"   => "processor.l3a.weight.cloud.coarseresolution",
				"value" => $coarseresolution ),
		array ( "key"   => "processor.l3a.weight.total.weightdatemin",
				"value" => $weightdatemin ),
		array ( "key"   => "processor.l3a.half_synthesis",
				"value" => $halfSynthesis )
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key"   => $cfg["key"], "value" => $cfg["value"] ));
		}
	}
	$json_config = json_encode( $fconfig );
	
	// generate json_param (skip parameters with empty values)
	$params = array (	"resolution" => $resolution,
						"input_products" => $input_products,
						"synthesis_date" => $synthDate
					);
	$json_param = json_encode( array_filter($params) );
	
	// set job name and description and save job
	$name = "l3a_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for ".$processor_short_name;
	insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l3b_lai------------------------------------------------------ */
elseif (isset ( $_POST ['l3b_lai'] )) {
	$processor_short_name = "l3b_lai";
	
	// default parameters
	$siteId         = $_POST ['siteId'];
	$input_products = $_POST ['inputFiles'];
	$resolution     = $_POST ['resolution'];
	$monolai        = $_POST ['lai'] == "monolai" ? 1 : 0;
	$reproc         = $_POST ['lai'] == "reproc"  ? 1 : 0;
	$fitted         = $_POST ['lai'] == "fitted"  ? 1 : 0;
	$bwr            = $_POST ['bwr'];
	$fwr            = $_POST ['fwr'];
	
	// advanced parameters
	$genmodel = $_POST ['genmodel'];
	
	$config = array (
		array ( "key"   => "processor.l3b.lai.localwnd.bwr",
				"value" => $bwr ),
		array ( "key"   => "processor.l3b.lai.localwnd.fwr",
				"value" => $fwr ),
		array ( "key"   => "processor.l3b.generate_models",
				"value" => $genmodel ),
		array ( "key"   => "processor.l3b.reprocess",
				"value" => $reproc ),
		array ( "key"   => "processor.l3b.fitted",
				"value" => $fitted ),
		array ( "key"   => "processor.l3b.mono_date_lai",
				"value" => $monolai )
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key"   => $cfg["key"], "value" => $cfg["value"] ));
		}
	}
	$json_config = json_encode( $fconfig );
	
	// generate json_param (skip parameters with empty values)
	$params = array (	"resolution"     => $resolution,
						"input_products" => $input_products
					);
	$json_param = json_encode( array_filter($params) );
	
	// set job name and description and save job
	$name = "l3b_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for ".$processor_short_name;
	insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l3e_pheno------------------------------------------------------ */
elseif (isset ( $_POST ['l3e_pheno'] )) {
	$processor_short_name = "l3e_pheno";
	
	// default parameters
	$siteId         = $_POST ['siteId'];
	$input_products = $_POST ['inputFiles'];
	$resolution     = $_POST ['resolution'];
	
	$json_config = json_encode ( array (
		array ( "key"   => "",
				"value" => "" )
	) );
	
	// generate json_param (skip parameters with empty values)
	$params = array (	"resolution"     => $resolution,
						"input_products" => $input_products
					);
	$json_param = json_encode( array_filter($params) );
	
	// set job name and description and save job
	$name = "l3e_pheno_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for ".$processor_short_name;
	insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l4a------------------------------------------------------ */
elseif (isset ( $_POST ['l4a'] ) || isset ( $_POST ['l4a_wo'] )) {
	$processor_short_name = "l4a";
	
	// default parameters
	$siteId         = $_POST ['siteId'];
	$input_products = $_POST ['inputFiles'];
	$resolution     = $_POST ['resolution'];
	$ratio          = $_POST ['ratio'];
	
	// advanced parameters
	$mission    = $_POST ['mission'];
	$field      = $_POST ['field'];
	$trm        = $_POST ['trm'];
	$radius     = $_POST ['radius'];
	$nbtrsample = $_POST ['nbtrsample'];
	$rseed      = $_POST ['rseed'];
	$window     = $_POST ['window'];
	$lmbd       = $_POST ['lmbd'];
	$weight     = $_POST ['weight'];
	$nbcomp     = $_POST ['nbcomp'];
	$spatialr   = $_POST ['spatialr'];
	$ranger     = $_POST ['ranger'];
	$minsize    = $_POST ['minsize'];
	$eroderad   = $_POST ['eroderad'];
	$alpha      = $_POST ['alpha'];
	$classifier = $_POST ['classifier'];
	$rfnbtrees  = $_POST ['rfnbtrees'];
	$rfmax      = $_POST ['rfmax'];
	$rfmin      = $_POST ['rfmin'];
	$minarea    = $_POST ['minarea'];
	
	$config = array (
		array ( "key"   => "processor.l4a.mission",
				"value" => $mission ),
		array ( "key"   => "processor.l4a.sample-ratio",
				"value" => $ratio ),
		array ( "key"   => "processor.l4a.temporal_resampling_mode",
				"value" => $trm ),
		array ( "key"   => "processor.l4a.radius",
				"value" => $radius ),
		array ( "key"   => "processor.l4a.training-samples-number",
				"value" => $nbtrsample ),
		array ( "key"   => "processor.l4a.random_seed",
				"value" => $rseed ),
		array ( "key"   => "processor.l4a.window",
				"value" => $window ),
		array ( "key"   => "processor.l4a.smoothing-lambda",
				"value" => $lmbd ),
		array ( "key"   => "processor.l4a.weight",
				"value" => $weight ),
		array ( "key"   => "processor.l4a.nbcomp",
				"value" => $nbcomp ),
		array ( "key"   => "processor.l4a.segmentation-spatial-radius",
				"value" => $spatialr ),
		array ( "key"   => "processor.l4a.range-radius",
				"value" => $ranger ),
		array ( "key"   => "processor.l4a.segmentation-minsize",
				"value" => $minsize ),
		array ( "key"   => "processor.l4a.erode-radius",
				"value" => $eroderad ),
		array ( "key"   => "processor.l4a.mahalanobis-alpha",
				"value" => $alpha ),
		array ( "key"   => "processor.l4a.classifier",
				"value" => $classifier ),
		array ( "key"   => "processor.l4a.classifier.field",
				"value" => $field ),
		array ( "key"   => "processor.l4a.classifier.rf.nbtrees",
				"value" => $rfnbtrees ),
		array ( "key"   => "processor.l4a.classifier.rf.max",
				"value" => $rfmax ),
		array ( "key"   => "processor.l4a.classifier.rf.min",
				"value" => $rfmin ),
		array ( "key"   => "processor.l4a.min-area",
				"value" => $minarea )
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key"   => $cfg["key"], "value" => $cfg["value"] ));
		}
	}
	$json_config = json_encode( $fconfig );
	
	$date = date_create();
	$timestamp = date_timestamp_get($date);
	if (isset($_POST['l4a'])) {
		// l4a with in-situ
		$upload        = upload_reference_polygons($siteId, $timestamp);
		$polygons_file = $upload['polygons_file'];
		$result        = $upload['result'];
		$message       = $upload['message'];
		if ($polygons_file) {
			$params = array (	"resolution"         => $resolution,
								"input_products"     => $input_products,
								"reference_polygons" => $polygons_file
							);
			$json_param = json_encode( array_filter($params), JSON_UNESCAPED_SLASHES );
			
			// set job name and description and save job
			$name = "l4a_processor" . date ( "m.d.y" );
			$description = "generated new configuration from site for ".$processor_short_name;
			insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
			redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
		} else {
			redirect_page("l4a", "NOK", $result." (".$message.")");
		}
	} else {
		// l4a w/o in-situ
		$raster_file = upload_reference_raster($siteId, $timestamp);
		$params = array (	"resolution"       => $resolution,
							"input_products"   => $input_products,
							"reference_raster" => $raster_file
						);
		$json_param = json_encode( array_filter($params), JSON_UNESCAPED_SLASHES );
		
		// set job name and description and save job
		$name = "l4a_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page("l4a_wo", "OK", "Your job has been successfully submitted!");
	}
} /* -------------------------------------------------------l4b------------------------------------------------------ */
elseif (isset ( $_POST ['l4b'] )) {
	$processor_short_name = "l4b";
	
	// default parameters
	$siteId         = $_POST ['siteId'];
	$resolution     = $_POST ['resolution'];
	$input_products = $_POST ['inputFiles'];
	$crop_mask      = $_POST ['cropMask'];
	$ratio          = $_POST ['ratio'];
	
	// advanced parameters
	$mission    = $_POST ['mission'];
	$field      = $_POST ['field'];
	$trm        = $_POST ['trm'];
	$rseed      = $_POST ['rseed'];
	$classifier = $_POST ['classifier'];
	$rfnbtrees  = $_POST ['rfnbtrees'];
	$rfmax      = $_POST ['rfmax'];
	$rfmin      = $_POST ['rfmin'];
	
	$config = array (
		array ( "key"   => "processor.l4b.mission",
				"value" => $mission ),
		array ( "key"   => "processor.l4b.sample-ratio",
				"value" => $ratio ),
		array ( "key"   => "processor.l4b.temporal_resampling_mode",
				"value" => $trm ),
		array ( "key"   => "processor.l4b.random_seed",
				"value" => $rseed ),
		array ( "key"   => "processor.l4b.classifier",
				"value" => $classifier ),
		array ( "key"   => "processor.l4b.classifier.field",
				"value" => $field ),
		array ( "key"   => "processor.l4b.classifier.rf.nbtrees",
				"value" => $rfnbtrees ),
		array ( "key"   => "processor.l4b.classifier.rf.max",
				"value" => $rfmax ),
		array ( "key"   => "processor.l4b.classifier.rf.min",
				"value" => $rfmin )
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key"   => $cfg["key"], "value" => $cfg["value"] ));
		}
	}
	$json_config = json_encode( $fconfig );
	
	// upload polygons
	$date          = date_create();
	$timestamp     = date_timestamp_get($date);
	$upload        = upload_reference_polygons($siteId, $timestamp);
	$polygons_file = $upload['polygons_file'];
	$result        = $upload['result'];
	$message       = $upload['message'];
	
	// generate json_param (skip parameters with empty values)
	if ($polygons_file) {
		$params = array (	"resolution"         => $resolution,
							"input_products"     => $input_products,
							"crop_mask"          => $crop_mask,
							"reference_polygons" => $polygons_file
						);
		$json_param = json_encode( array_filter($params), JSON_UNESCAPED_SLASHES );
		
		// set job name and description and save job
		$name = "l4b_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
	} else {
		redirect_page($processor_short_name, "NOK", $result." (".$message.")");
	}
}
?>
