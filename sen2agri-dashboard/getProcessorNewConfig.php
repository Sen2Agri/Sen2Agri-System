<?php
session_start();
require_once ("ConfigParams.php");

function redirect_page($processor_short_name, $status, $message) {
	$_SESSION['processor'] = $processor_short_name;
	$_SESSION['proc_div']  = isset($_POST ['proc_div']) ? $_POST ['proc_div'] : "foo";
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
    $dbconn = pg_connect( ConfigParams::getConnection() ) or die ( "Could not connect" );
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
		exec('scripts/check_shp.py -f '.$shp_file, $output, $ret);

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
    $db = pg_connect( ConfigParams::getConnection() ) or die ( "Could not connect" );

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
			$url= ConfigParams::$SERVICES_URL."/NotifyOrchestrator";

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

// sen2agri
if (ConfigParams::isSen2Agri()) {
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
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
	} /* -------------------------------------------------------l3b_lai------------------------------------------------------ */
	elseif (isset ( $_POST ['l3b_lai'] ) || isset ( $_POST ['l3b'] )) {
		$processor_short_name = "l3b";

		// default parameters
		$siteId         = $_POST ['siteId'];
		$input_products = $_POST ['inputFiles'];
		$resolution     = $_POST ['resolution'];

		// advanced parameters
		$genmodel = $_POST ['genmodel'];

		$config = array (
			array ( "key"   => "processor.l3b.generate_models",
					"value" => $genmodel )
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
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
	}
/* -------------------------------------------------------s2a_l3c------------------------------------------------------ */
	elseif (isset ( $_POST ['s2a_l3c'] )) {
		$processor_short_name = "s2a_l3c";

		// default parameters
		$siteId         = $_POST ['siteId'];
		$input_products = $_POST ['inputFiles'];
		$resolution     = $_POST ['resolution'];
		$bwr            = $_POST ['bwr'];
		$fwr            = $_POST ['fwr'];

		$config = array (
			array ( "key"   => "processor.l3b.lai.localwnd.bwr",
					"value" => $bwr ),
			array ( "key"   => "processor.l3b.lai.localwnd.fwr",
					"value" => $fwr )
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
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
	}/* -------------------------------------------------------s2a_l3d------------------------------------------------------ */
	elseif (isset ( $_POST ['s2a_l3d'] )) {
		$processor_short_name = "s2a_l3d";

		// default parameters
		$siteId         = $_POST ['siteId'];
		$input_products = $_POST ['inputFiles'];
		$resolution     = $_POST ['resolution'];
		$config = array ();

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
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
	}
    /* -------------------------------------------------------l3e_pheno------------------------------------------------------ */
	elseif (isset ( $_POST ['l3e_pheno'] ) || isset ( $_POST ['l3e'] )) {
		$processor_short_name = "l3e";

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
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
	} /* -------------------------------------------------------l4a------------------------------------------------------ */
	elseif (isset ( $_POST ['l4a'] ) || isset ( $_POST ['l4a_wo'] )) {
		$processor_short_name = "l4a";

		// default parameters
		$siteId         = $_POST ['siteId'];
		$input_products = $_POST ['inputFiles'];
		$resolution     = $_POST ['resolution'];
		$ratio          = $_POST ['ratio'];

		// advanced parameters
		$field      = $_POST ['field'];
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
			array ( "key"   => "processor.l4a.sample-ratio",
					"value" => $ratio ),
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
			$name = $processor_short_name . "_processor" . date ( "m.d.y" );
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
		$field      = $_POST ['field'];
		$rseed      = $_POST ['rseed'];
		$classifier = $_POST ['classifier'];
		$rfnbtrees  = $_POST ['rfnbtrees'];
		$rfmax      = $_POST ['rfmax'];
		$rfmin      = $_POST ['rfmin'];

		$config = array (
			array ( "key"   => "processor.l4b.sample-ratio",
					"value" => $ratio ),
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
								"reference_polygons" => $polygons_file
							);
			if (!empty($crop_mask)) {
				$params["crop_mask"] = $crop_mask;
			}
			$json_param = json_encode( array_filter($params), JSON_UNESCAPED_SLASHES );

			// set job name and description and save job
			$name = $processor_short_name . "_processor" . date ( "m.d.y" );
			$description = "generated new configuration from site for ".$processor_short_name;
			insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
			redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
		} else {
			redirect_page($processor_short_name, "NOK", $result." (".$message.")");
		}
	}
}
// sen4cap
else {
/* -------------------------------------------------------l3b_lai------------------------------------------------------ */
	if (isset ( $_POST ['l3b_lai'] ) || isset ( $_POST ['l3b'] )) {
        // TODO: A lot of Duplicated code with the previous ifs. To be made more generic
		$processor_short_name = "l3b";

		// default parameters
		$siteId         = $_POST ['siteId'];
		$input_products = $_POST ['inputFiles'];
		$resolution     = $_POST ['resolution'];

		// advanced parameters
		$genmodel = $_POST ['genmodel'];

		$config = array (
			array ( "key"   => "processor.l3b.generate_models",
					"value" => $genmodel )
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
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		insertjob ( $name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config );
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!");
    }
	elseif (isset($_POST['s4c_l4a'])) {
        // TODO: A lot of Duplicated code with the previous ifs. To be made more generic
		$processor_short_name = "s4c_l4a";
        $processor_cfg_prefix = "processor." . $processor_short_name . ".";
		// set job name and description and save job
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		
        // print_r($_POST);
        // exit();
		// default parameters
		$siteId         = $_POST['siteId'];
		$input_products = $_POST['inputFiles'];

		// advanced parameters	- dynamically get the 
        $fconfig = array();        
        foreach ($_POST as $key => $value) {
            if ($value != "") {
                $suffix = "_".$processor_short_name;
                if (endsWith($key, $suffix))  {
                    $realKeyName = substr($key, 0, strrpos($key, $suffix));
                    $cfgKeyToSend = $processor_cfg_prefix . $realKeyName;
                    array_push($fconfig, array ( "key"   => $cfgKeyToSend, "value" => $value ));
                }
            }
        }
		$json_config = json_encode( $fconfig );
		
		// generate json_param (skip parameters with empty values)
		$params = array ("input_products" => $input_products);
		$json_param = json_encode(array_filter($params));
		
		insertjob($name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config);
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!", $params);
	}
	elseif (isset($_POST['s4c_l4b'])) {
        // TODO: A lot of Duplicated code with the previous ifs. To be made more generic
		$processor_short_name = "s4c_l4b";
        $processor_cfg_prefix = "processor." . $processor_short_name . ".";
		// set job name and description and save job
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		
		// default parameters
		$siteId     = $_POST['siteId'];
		$input_COHE = $_POST['inputFiles_COHE'];
		$input_AMP  = $_POST['inputFiles_AMP'];
		$input_NDVI = $_POST['inputFiles_NDVI'];
		
		// advanced parameters	- dynamically get the 
        $fconfig = array();        
        foreach ($_POST as $key => $value) {
            if ($value != "") {
                $suffix = "_".$processor_short_name;
                if (endsWith($key, $suffix))  {
                    $realKeyName = substr($key, 0, strrpos($key, $suffix));
                    $cfgKeyToSend = $processor_cfg_prefix . $realKeyName;
                    array_push($fconfig, array ( "key"   => $cfgKeyToSend, "value" => $value ));
                }
            }
        }
		
        $json_config = json_encode( $fconfig );
		
		// generate json_param (skip parameters with empty values)
		$params = array ("input_COHE" => $input_COHE, "input_AMP"  => $input_AMP, "input_NDVI" => $input_NDVI);
		$json_param = json_encode(array_filter($params));
		
		insertjob($name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config);
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!", $params);
	}
	elseif (isset($_POST['s4c_l4c'])) {
        // TODO: A lot of Duplicated code with the previous ifs. To be made more generic
		$processor_short_name = "s4c_l4c";
        $processor_cfg_prefix = "processor." . $processor_short_name . ".";
		// set job name and description and save job
		$name = $processor_short_name . "_processor" . date ( "m.d.y" );
		$description = "generated new configuration from site for ".$processor_short_name;
		
		// default parameters
		$siteId     = $_POST['siteId'];
		$input_COHE = $_POST['inputFiles_COHE'];
		$input_AMP  = $_POST['inputFiles_AMP'];
		$input_NDVI = $_POST['inputFiles_NDVI'];
        $s2_tiles = $_POST['S2Tiles'];
        $l8_tiles = $_POST['L8Tiles'];
        $s2_l8_tiles = $s2_tiles . ((strlen($s2_tiles) > 0 && strlen($l8_tiles) > 0) ? "," : "") . $l8_tiles; 
		
        // print_r($s2_l8_tiles);
        // exit();

		// advanced parameters	- dynamically get the 
        $fconfig = array();        
        foreach ($_POST as $key => $value) {
            if ($value != "") {
                $suffix = "_".$processor_short_name;
                if (endsWith($key, $suffix))  {
                    $realKeyName = substr($key, 0, strrpos($key, $suffix));
                    $cfgKeyToSend = $processor_cfg_prefix . $realKeyName;
                    array_push($fconfig, array ( "key"   => $cfgKeyToSend, "value" => $value ));
                }
            }
        }
		
        $json_config = json_encode( $fconfig );
		
		// generate json_param (skip parameters with empty values)
		$params = array ("input_COHE" => $input_COHE, "input_AMP"  => $input_AMP, "input_NDVI" => $input_NDVI, "s2_l8_tiles" => $s2_l8_tiles);
		$json_param = json_encode(array_filter($params));
		
		insertjob($name, $description, $processor_short_name, $siteId, 2, $json_param, $json_config);
		redirect_page($processor_short_name, "OK", "Your job has been successfully submitted!", $params);
	}
	else {
		echo "Under Construction";
	}
}
?>
