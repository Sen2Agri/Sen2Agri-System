<?php
require_once ('ConfigParams.php');

function upload_reference_polygons($site_id) {
	$dbconn = pg_connect ( 'host=sen2agri-dev port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
	
	$rows = pg_query($dbconn, "SELECT key, value FROM sp_get_parameters('site.upload_path') WHERE site_id IS NULL")
			or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[1];
	$upload_target_dir = str_replace("{user}", "", $result);
	
	$upload_target_dir = $upload_target_dir . $site_id . "/";
	echo "</br>Upload reference polygons to: " . $upload_target_dir;
	
	if (!is_dir($upload_target_dir)) {
		mkdir($upload_target_dir, 0755, true);
	}
	
	if($_FILES["refp"]["name"]) {
		$filename = $_FILES["refp"]["name"];
		$source = $_FILES["refp"]["tmp_name"];
		$type = $_FILES["refp"]["type"];
		
		$fname = explode(".", $filename);
		$accepted_types = array('application/zip', 'application/x-zip-compressed', 'multipart/x-zip', 'application/x-compressed');
		foreach($accepted_types as $mime_type) {
			if($mime_type == $type) {
				$okay = true;
				break;
			}
		}
		
		$continue = strtolower($fname[1]) == 'zip' ? true : false;
		if(!$continue) {
			$message = "The file you are trying to upload is not a .zip file. Please try again.";
		}
		
		$target_path = $upload_target_dir . $filename;  // change this to the correct site path
		if(move_uploaded_file($source, $target_path)) {
			$zip = new ZipArchive();
			$x = $zip->open($target_path);
			if ($x === true) {
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

function insertjob($name, $description, $processor_id, $site_id, $start_type_id, $parameters, $configuration) {
	$db = pg_connect ( 'host=sen2agri-dev port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );

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
	 
	 header("Location: config.php");
	 exit();
}

if (isset ( $_POST ['l3a'] )) {
	$siteId = $_POST ['siteId'];
	/*
	 * $sentinel2Tiles = $_POST ['sentinel2Tiles'];
	 * $landsatTiles = $_POST ['landsatTiles'];
	 */
	
	$inputFiles = $_POST ['inputFiles'];
	$resolution = $_POST ['resolution'];
	$synthDate = $_POST ['synthDate'];
	$halfSynthesis = $_POST ['halfSynthesis'];
	
	$maxaot = $_POST ['maxaot'];
	$minweight = $_POST ['minweight'];
	$maxweight = $_POST ['maxweight'];
	
	$sigmasmall = $_POST ['sigmasmall'];
	$sigmalarge = $_POST ['sigmalarge'];
	$coarseresolution = $_POST ['coarseresolution'];
	$weightdatemin = $_POST ['weightdatemin'];
	
	$json_config = json_encode ( array (
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
			) 
	) );
	
	$json_param = json_encode ( array (
			"resolution" => $resolution,
			"input_products" => $inputFiles,
			"synthesis-date" => $synthDate,
			"half-synthesis" => $halfSynthesis 
	) );
	
	echo $json_param;
	echo "<br>";
	echo $json_config;
	echo "<br>";
	
	// job name
	$name = "l3a_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l3a";
	
	insertjob ( $name, $description, 2, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l3b_lai------------------------------------------------------ */
elseif (isset ( $_POST ['l3b_lai'] )) {
	$siteId = $_POST ['siteId'];
	
	$input_products = $_POST ['inputFiles'];
	$resolution = $_POST ['resolution'];
	$genmodel = $_POST ['genmodel'];
	$reproc = $_POST ['reproc'];
	$fitted = $_POST ['fitted'];
	
	$bwr = $_POST ['bwr'];
	$fwr = $_POST ['fwr'];
	
	/*
	 * is not sure if those will be keept
	 *
	 * $modelsfolder = $_POST ['modelsfolder'];
	 * $rsrcfgfile = $_POST ['rsrcfgfile'];
	 */
	
	$json_config = json_encode ( array (
			array (
					"key" => "processor.l3b.lai.localwnd.bwr",
					"value" => $bwr 
			),
			array (
					"key" => "processor.l3b.lai.localwnd.fwr",
					"value" => $fwr 
			) 
	) );
	
	$json_param = json_encode ( array (
			"input_products" => $input_products,
			"resolution" => $resolution,
			"genmodel" => $genmodel,
			"reproc" => $reproc,
			"fitted" => $fitted 
	) );
	
	echo $json_config;
	echo "<br>";
	echo $json_param;
	
	// job name
	$name = "l3b_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l3b_lai";
	
	insertjob ( $name, $description, 3, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l3b_pheno------------------------------------------------------ */
elseif (isset ( $_POST ['l3b_pheno'] )) {
	$siteId = $_POST ['siteId'];
	
	$input_products = $_POST ['inputFiles'];
	$resolution = $_POST ['resolution'];
	
	$json_config = json_encode ( array (
			array (
					"key" => "",
					"value" => "" 
			) 
	) );
	
	$json_param = json_encode ( array (
			"input_products" => $input_products,
			"resolution" => $resolution 
	) );
	
	echo $json_config;
	echo "<br>";
	echo $json_param;
	
	// job name
	$name = "l3b_pheno_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l3b_lai";
	
	insertjob ( $name, $description, 7, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l4a------------------------------------------------------ */
elseif (isset ( $_POST ['l4a'] )) {
	
	$siteId = $_POST ['siteId'];
	
	$input_products = $_POST ['inputFiles'];
	$resolution = $_POST ['resolution'];
	
	$mission = $_POST ['mission'];
	$refp = $_POST ['refp'];
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
	$refr = $_POST ['refr'];
	$eroderad = $_POST ['eroderad'];
	$alpha = $_POST ['alpha'];
	$classifier = $_POST ['classifier'];
	$field = $_POST ['field'];
	$rfnbtrees = $_POST ['rfnbtrees'];
	$rfmax = $_POST ['rfmax'];
	$rfmin = $_POST ['rfmin'];
	/*
	 * TODO
	 * $svmk = $_POST ['svmk'];
	 * $svmopt = $_POST ['svmopt'];
	 * end TODO
	 */
	$minarea = $_POST ['minarea'];
	$pixsize = $_POST ['pixsize'];
	
	$json_config = json_encode ( array (
			array (
					"key" => "processor.l4a.mission",
					"value" => $mission 
			),
			array (
					"key" => "processor.l4a.refp",
					"value" => $refp 
			),
			array (
					"key" => "processor.l4a.ratio",
					"value" => $ratio 
			),
			array (
					"key" => "processor.l4a.trm",
					"value" => $trm 
			),
			array (
					"key" => "processor.l4a.radius",
					"value" => $radius 
			),
			array (
					"key" => "processor.l4a.nbtrsample",
					"value" => $nbtrsample 
			),
			array (
					"key" => "processor.l4a.rseed",
					"value" => $rseed 
			),
			array (
					"key" => "processor.l4a.window",
					"value" => $window 
			),
			array (
					"key" => "processor.l4a.lmbd",
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
					"key" => "processor.l4a.spatialr",
					"value" => $spatialr 
			),
			array (
					"key" => "processor.l4a.ranger",
					"value" => $ranger 
			),
			array (
					"key" => "processor.l4a.minsize",
					"value" => $minsize 
			),
			array (
					"key" => "processor.l4a.refr",
					"value" => $refr 
			),
			array (
					"key" => "processor.l4a.eroderad",
					"value" => $eroderad 
			),
			array (
					"key" => "processor.l4a.alpha",
					"value" => $alpha 
			),
			array (
					"key" => "processor.l4a.classifier",
					"value" => $classifier 
			),
			array (
					"key" => "processor.l4a.field",
					"value" => $field 
			),
			array (
					"key" => "processor.l4a.rfnbtrees",
					"value" => $rfnbtrees 
			),
			array (
					"key" => "processor.l4a.rfmax",
					"value" => $rfmax 
			),
			array (
					"key" => "processor.l4a.rfmin",
					"value" => $rfmin 
			),
			array (
					"key" => "processor.l4a.minarea",
					"value" => $minarea 
			),
			array (
					"key" => "processor.l4a.pixsize",
					"value" => $pixsize 
			) 
	) );
	
	$json_param = json_encode ( array (
			"input_products" => $input_products,
			"resolution" => $resolution 
	) );
	echo $json_config;
	echo "<br>";
	echo $json_param;
	
	// job name
	$name = "l4a_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l4a";
	
	upload_reference_polygons($siteId);
	insertjob ( $name, $description, 4, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l4b------------------------------------------------------ */
elseif (isset ( $_POST ['l4b'] )) {

	$siteId = $_POST ['siteId'];
	
	$input_products = $_POST ['inputFiles'];
	$crop_mask = $_POST ['cropMasks'];
	$resolution = $_POST ['resolution'];
	
	//$refp = $_POST ['refp'];
	$refp = $_FILES["refp"]["name"];
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
	/*
	 * TODO
	 * $svmk = $_POST ['svmk'];
	 * $svmopt = $_POST ['svmopt'];
	 */
	
	$json_config = json_encode ( array (
			array (
					"key" => "processor.l4b.mission",
					"value" => $mission 
			),
			array (
					"key" => "processor.l4b.refp",
					"value" => $refp 
			),
			array (
					"key" => "processor.l4b.ratio",
					"value" => $ratio 
			),
			array (
					"key" => "processor.l4b.trm",
					"value" => $trm 
			),
			array (
					"key" => "processor.l4b.radius",
					"value" => $radius 
			),
			array (
					"key" => "processor.l4b.rseed",
					"value" => $rseed 
			),
			array (
					"key" => "processor.l4b.classifier",
					"value" => $classifier 
			),
			array (
					"key" => "processor.l4b.field",
					"value" => $field 
			),
			array (
					"key" => "processor.l4b.rfnbtrees",
					"value" => $rfnbtrees 
			),
			array (
					"key" => "processor.l4b.rfmax",
					"value" => $rfmax 
			),
			array (
					"key" => "processor.l4b.rfmin",
					"value" => $rfmin 
			)
	) );
	
	$json_param = json_encode ( array (
			"input_products" => $input_products,
			"crop_mask" => $crop_mask,
			"resolution" => $resolution 
	) );
	
	echo $json_config;
	echo "<br>";
	echo $json_param;
	
	// job name
	$name = "l4b_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l4b";
	
	upload_reference_polygons($siteId);
	insertjob ( $name, $description, 5, $siteId, 2, $json_param, $json_config );
}


?>