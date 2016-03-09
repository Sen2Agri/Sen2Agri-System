<?php
session_start();
require_once ("ConfigParams.php");

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
	
	header("Location: config.php"); /* Redirect */
	exit();
}

if (isset ( $_POST ['l3a'] )) {
	$siteId = $_POST ['siteId'];
	/*
	$sentinel2Tiles = $_POST ['sentinel2Tiles'];
	$landsatTiles = $_POST ['landsatTiles'];
	*/
	
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
			),
			array (
					"key" => "processor.l3a.half_synthesis",
					"value" => $halfSynthesis
			)
	) );
	
	$params = array (
		"resolution" => $resolution,
		"input_products" => $input_products,
		"synthesis_date" => $synthDate);
	$json_param = json_encode( array_filter($params) );
	
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
	
	$json_config = json_encode ( array (
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
	) );
	
	$params = array (
		"resolution" => $resolution,
		"input_products" => $input_products);
	$json_param = json_encode( array_filter($params) );
	
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
	
	$params = array (
		"resolution" => $resolution,
		"input_products" => $input_products);
	$json_param = json_encode( array_filter($params) );
	
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
	
	insertjob ( $name, $description, 4, $siteId, 2, $json_param, $json_config );
} /* -------------------------------------------------------l4b------------------------------------------------------ */
elseif (isset ( $_POST ['l4b'] )) {
	$siteId = $_POST ['siteId'];
	
	$input_products = $_POST ['inputFiles'];
	$resolution = $_POST ['resolution'];
	
	$refp = $_POST ['refp'];
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
			"resolution" => $resolution 
	) );
	
	echo $json_config;
	echo "<br>";
	echo $json_param;
	
	// job name
	$name = "l4b_processor" . date ( "m.d.y" );
	$description = "generated new configuration from site for l4b";
	
	insertjob ( $name, $description, 5, $siteId, 2, $json_param, $json_config );
}


?>