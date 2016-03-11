<?php
session_start();
require_once ("ConfigParams.php");

function endsWith( $str, $sub ) {
   return ( substr( $str, strlen( $str ) - strlen( $sub ) ) === $sub );
}

function upload_reference_polygons($site_id) {
	$dbconn = pg_connect ( 'host='.ConfigParams::$SERVER_NAME.' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
	
	$rows = pg_query($dbconn, "SELECT key, value FROM sp_get_parameters('site.upload_path') WHERE site_id IS NULL") or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[1];
	$upload_target_dir = str_replace("{user}", "", $result);
	$upload_target_dir = $upload_target_dir . $site_id . "/";
	
	if (!is_dir($upload_target_dir)) {
		mkdir($upload_target_dir, 0755, true);
	}
	
	$shp_file = FALSE;
	if($_FILES["refp"]["name"]) {
		$filename = $_FILES["refp"]["name"];
		$source = $_FILES["refp"]["tmp_name"];
		$type = $_FILES["refp"]["type"];
		
		$zip_file = false;
		$accepted_types = array('application/zip', 'application/x-zip-compressed', 'multipart/x-zip', 'application/x-compressed');
		foreach($accepted_types as $mime_type) {
			if($mime_type == $type) {
				$zip_file = true;
				break;
			}
		}
		
		if ($zip_file) {
			$target_path = $upload_target_dir . $filename;  // change this to the correct site path
			if(move_uploaded_file($source, $target_path)) {
				$zip = new ZipArchive();
				$x = $zip->open($target_path);
				if ($x === true) {
					for ($i = 0; $i < $zip->numFiles; $i++) {
						$filename = $zip->getNameIndex($i);
						if (endsWith($filename, '.shp')) {
							$shp_file = $upload_target_dir . $filename;
						}
					}
					$zip->extractTo($upload_target_dir); // change this to the correct site path
					$zip->close();
					unlink($target_path);
				}
				$message = "Your .zip file was uploaded and unpacked.";
			} else {
				$message = "There was a problem with the upload. Please try again.";
			}
		}
	}
	return $shp_file;
}

function upload_reference_raster($site_id) {
	$dbconn = pg_connect ( 'host='.ConfigParams::$SERVER_NAME.' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
	
	$rows = pg_query($dbconn, "SELECT key, value FROM sp_get_parameters('site.upload_path') WHERE site_id IS NULL") or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[1];
	$upload_target_dir = str_replace("{user}", "", $result);
	$upload_target_dir = $upload_target_dir . $site_id . "/";
	
	if (!is_dir($upload_target_dir)) {
		mkdir($upload_target_dir, 0755, true);
	}
	
	$shp_file = FALSE;
	if($_FILES["refr"]["name"]) {
		$filename = $_FILES["refr"]["name"];
		$source = $_FILES["refr"]["tmp_name"];
		$type = $_FILES["refr"]["type"];
		
		$img_file = false;
		$accepted_types = array('image/tiff', 'image/png', 'image/jp2');
		foreach($accepted_types as $mime_type) {
			if($mime_type == $type) {
				$img_file = true;
				break;
			}
		}
		
		if ($img_file) {
			$target_path = $upload_target_dir . $filename;  // change this to the correct site path
			if(move_uploaded_file($source, $target_path)) {
				$shp_file = $target_path;
			} else {
				$message = "There was a problem with the upload. Please try again.";
			}
		}
	}
	return $shp_file;
}

function insertjob($name, $description, $processor_id, $site_id, $start_type_id, $parameters, $configuration) {
	$db = pg_connect ( 'host='.ConfigParams::$SERVER_NAME.' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
	
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
	
	// if job is for processor l4a or l4b set a session variable
	// which will be used after redirection to custom jobs page
	switch ($processor_id) {
		case 4: $_SESSION['processor'] = 'l4a'; break;
		case 5: $_SESSION['processor'] = 'l4b'; break;
		default: ;
	}
	
	// redirect to custom jobs page
	$referer = $_SERVER['HTTP_REFERER'];
	header("Location: $referer");
	exit();
}

if (isset ( $_POST ['l3a'] )) {
	$siteId = $_POST ['siteId'];
	
	$resolution = $_POST ['resolution'];
	$input_products = $_POST ['inputFiles'];
	$synthDate = $_POST ['synthDate'];
	
	$maxaot = $_POST ['maxaot'];
	$minweight = $_POST ['minweight'];
	$maxweight = $_POST ['maxweight'];
	$sigmasmall = $_POST ['sigmasmall'];
	$sigmalarge = $_POST ['sigmalarge'];
	$coarseresolution = $_POST ['coarseresolution'];
	$weightdatemin = $_POST ['weightdatemin'];
	$halfSynthesis = $_POST ['halfSynthesis'];
	
	$config = array (
			array (
					"key" => "processor.l3a.weight.aot.maxaot",
					"value" => $maxaot
			),
			array (
					"key" => "processor.l3a.weight.aot.minweight",
					"value" => $minweight
			),
			array (
					"key" => "processor.l3a.weight.aot.maxweight",
					"value" => $maxweight
			),
			array (
					"key" => "processor.l3a.weight.cloud.sigmasmall",
					"value" => $sigmasmall
			),
			array (
					"key" => "processor.l3a.weight.cloud.sigmalarge",
					"value" => $sigmalarge
			),
			array (
					"key" => "processor.l3a.weight.cloud.coarseresolution",
					"value" => $coarseresolution
			),
			array (
					"key" => "processor.l3a.weight.total.weightdatemin",
					"value" => $weightdatemin
			),
			array (
					"key" => "processor.l3a.half_synthesis",
					"value" => $halfSynthesis
			)
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key" => $cfg["key"], "value" => $cfg["value"] ));
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
	$description = "generated new configuration from site for l3a";
	insertjob ( $name, $description, 2, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l3b_lai------------------------------------------------------ */
elseif (isset ( $_POST ['l3b_lai'] )) {
	$siteId = $_POST ['siteId'];
	
	$resolution = $_POST ['resolution'];
	$input_products = $_POST ['inputFiles'];
	
	$bwr = $_POST ['bwr'];
	$fwr = $_POST ['fwr'];
	$genmodel = $_POST ['genmodel'];
	$reproc = $_POST ['reproc'];
	$fitted = $_POST ['fitted'];
	
	$config = array (
			array (
					"key" => "processor.l3b.lai.localwnd.bwr",
					"value" => $bwr
			),
			array (
					"key" => "processor.l3b.lai.localwnd.fwr",
					"value" => $fwr
			),
			array (
					"key" => "processor.l3b.generate_models",
					"value" => $genmodel
			),
			array (
					"key" => "processor.l3b.reprocess",
					"value" => $reproc
			),
			array (
					"key" => "processor.l3b.fitted",
					"value" => $fitted
			)
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key" => $cfg["key"], "value" => $cfg["value"] ));
		}
	}
	$json_config = json_encode( $fconfig );
	
	// generate json_param (skip parameters with empty values)
	$params = array (	"resolution" => $resolution,
						"input_products" => $input_products
					);
	$json_param = json_encode( array_filter($params) );
	
	// set job name and description and save job
	$name = "l3b_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l3b_lai";
	insertjob ( $name, $description, 3, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l3b_pheno------------------------------------------------------ */
elseif (isset ( $_POST ['l3b_pheno'] )) {
	$siteId = $_POST ['siteId'];
	
	$resolution = $_POST ['resolution'];
	$input_products = $_POST ['inputFiles'];
	
	$json_config = json_encode ( array (
			array (
					"key" => "",
					"value" => ""
			) 
	) );
	
	// generate json_param (skip parameters with empty values)
	$params = array (	"resolution" => $resolution,
						"input_products" => $input_products
					);
	$json_param = json_encode( array_filter($params) );
	
	// set job name and description and save job
	$name = "l3b_pheno_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l3b_lai";
	insertjob ( $name, $description, 7, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l4a------------------------------------------------------ */
elseif (isset ( $_POST ['l4a'] )) {
	$siteId = $_POST ['siteId'];
	
	$resolution = $_POST ['resolution'];
	$input_products = $_POST ['inputFiles'];
	
	$mission = $_POST ['mission'];
	$ratio = $_POST ['ratio'];
	$trm = $_POST ['trm'];
	$radius = $_POST ['radius'];
	$nbtrsample = $_POST ['nbtrsample'];
	$rseed = $_POST ['rseed'];
	$window = $_POST ['window'];
	$lmbd = $_POST ['lmbd'];
	$weight = $_POST ['weight'];
	$nbcomp = $_POST ['nbcomp'];
	$spatialr = $_POST ['spatialr'];
	$ranger = $_POST ['ranger'];
	$minsize = $_POST ['minsize'];
	$eroderad = $_POST ['eroderad'];
	$alpha = $_POST ['alpha'];
	$classifier = $_POST ['classifier'];
	$field = $_POST ['field'];
	$rfnbtrees = $_POST ['rfnbtrees'];
	$rfmax = $_POST ['rfmax'];
	$rfmin = $_POST ['rfmin'];
	$minarea = $_POST ['minarea'];
	
	$config = array (
			array (
					"key" => "processor.l4a.mission",
					"value" => $mission
			),
			array (
					"key" => "processor.l4a.sample-ratio",
					"value" => $ratio
			),
			array (
					"key" => "processor.l4a.temporal_resampling_mode",
					"value" => $trm
			),
			array (
					"key" => "processor.l4a.radius",
					"value" => $radius
			),
			array (
					"key" => "processor.l4a.training-samples-number",
					"value" => $nbtrsample
			),
			array (
					"key" => "processor.l4a.random_seed",
					"value" => $rseed
			),
			array (
					"key" => "processor.l4a.window",
					"value" => $window
			),
			array (
					"key" => "processor.l4a.smoothing-lambda",
					"value" => $lmbd
			),
			array (
					"key" => "processor.l4a.weight",
					"value" => $weight
			),
			array (
					"key" => "processor.l4a.nbcomp",
					"value" => $nbcomp
			),
			array (
					"key" => "processor.l4a.segmentation-spatial-radius",
					"value" => $spatialr
			),
			array (
					"key" => "processor.l4a.range-radius",
					"value" => $ranger
			),
			array (
					"key" => "processor.l4a.segmentation-minsize",
					"value" => $minsize
			),
			array (
					"key" => "processor.l4a.erode-radius",
					"value" => $eroderad
			),
			array (
					"key" => "processor.l4a.mahalanobis-alpha",
					"value" => $alpha
			),
			array (
					"key" => "processor.l4a.classifier",
					"value" => $classifier
			),
			array (
					"key" => "processor.l4a.classifier.field",
					"value" => $field
			),
			array (
					"key" => "processor.l4a.classifier.rf.nbtrees",
					"value" => $rfnbtrees
			),
			array (
					"key" => "processor.l4a.classifier.rf.max",
					"value" => $rfmax
			),
			array (
					"key" => "processor.l4a.classifier.rf.min",
					"value" => $rfmin
			),
			array (
					"key" => "processor.l4a.min-area",
					"value" => $minarea
			)
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key" => $cfg["key"], "value" => $cfg["value"] ));
		}
	}
	$json_config = json_encode( $fconfig );
	
	// generate json_param (skip parameters with empty values)
	$polygons_file = upload_reference_polygons($siteId);
	$raster_file = false;
	if (!$polygons_file) {
		$raster_file = upload_reference_raster($siteId);
	}
	$params = array (	"resolution" => $resolution,
						"input_products" => $input_products,
						"reference_polygons" => $polygons_file,
						"reference_raster" => $raster_file
					);
	$json_param = json_encode( array_filter($params), JSON_UNESCAPED_SLASHES );
	
	// set job name and description and save job
	$name = "l4a_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l4a";
	insertjob ( $name, $description, 4, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l4b------------------------------------------------------ */
elseif (isset ( $_POST ['l4b'] )) {
	$siteId = $_POST ['siteId'];
	
	$input_products = $_POST ['inputFiles'];
	$crop_mask = $_POST ['cropMask'];
	$resolution = $_POST ['resolution'];
	
	$mission = $_POST ['mission'];
	$ratio = $_POST ['ratio'];
	$trm = $_POST ['trm'];
	$radius = $_POST ['radius'];
	$rseed = $_POST ['rseed'];
	$classifier = $_POST ['classifier'];
	$field = $_POST ['field'];
	$rfnbtrees = $_POST ['rfnbtrees'];
	$rfmax = $_POST ['rfmax'];
	$rfmin = $_POST ['rfmin'];
	
	$config = array (
			array (
					"key" => "processor.l4b.mission",
					"value" => $mission
			),
			array (
					"key" => "processor.l4b.sample-ratio",
					"value" => $ratio
			),
			array (
					"key" => "processor.l4b.temporal_resampling_mode",
					"value" => $trm
			),
			array (
					"key" => "processor.l4b.radius",
					"value" => $radius
			),
			array (
					"key" => "processor.l4b.random_seed",
					"value" => $rseed
			),
			array (
					"key" => "processor.l4b.classifier",
					"value" => $classifier
			),
			array (
					"key" => "processor.l4b.classifier.field",
					"value" => $field
			),
			array (
					"key" => "processor.l4b.classifier.rf.nbtrees",
					"value" => $rfnbtrees
			),
			array (
					"key" => "processor.l4b.classifier.rf.max",
					"value" => $rfmax
			),
			array (
					"key" => "processor.l4b.classifier.rf.min",
					"value" => $rfmin
			) 
	);
	
	// generate json_config (skip configuration parameters with empty values)
	$fconfig = array();
	foreach($config as $cfg) {
		if ($cfg["value"] != "") {
			array_push($fconfig,  array ( "key" => $cfg["key"], "value" => $cfg["value"] ));
		}
	}
	$json_config = json_encode( $fconfig );
	
	// generate json_param (skip parameters with empty values)
	$polygons_file = upload_reference_polygons($siteId);
	$params = array (	"resolution" => $resolution,
						"input_products" => $input_products,
						"crop_mask" => $crop_mask,
						"reference_polygons" => $polygons_file
					);
	$json_param = json_encode( array_filter($params), JSON_UNESCAPED_SLASHES );
	
	// set job name and description and save job
	$name = "l4b_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l4b";
	insertjob ( $name, $description, 5, $siteId, 2, $json_param, $json_config );
}

?>