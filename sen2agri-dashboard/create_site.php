<?php
include 'master.php';

class UploadFileDescriptor {
	public $id;
	public $descr;
	public $dbUploadDirKey; // the upload root dir
	public $uploadRelPath;  // relative path starting from $dbUploadDirKey where to upload the file.
							// It can be left empty and in this case the file is uploaded in the root dir
	public $expectedUploadFileExt;
	public $fileExt;
	public $addParams;
}

class PostUploadCmd {
	public $triggerUpDescrIds;
	public $cmd;
	public $params;
	public $isAsyncCmd;
}

function getUploadFileDescriptorArray() {
	$uploadFileDescriptorArray = array();
	if (ConfigParams::isSen2Agri()) {
		$descr = new UploadFileDescriptor();
		$descr->id = "Insitu";
		$descr->descr = "Insitu data";
		$descr->dbUploadDirKey = "processor.l4a.reference_data_dir";
		$descr->uploadRelPath = "";
		$descr->expectedUploadFileExt = ".zip";
		$descr->fileExt = "shp";
		$uploadFileDescriptorArray[] = $descr;

		$descr = new UploadFileDescriptor();
		$descr->id = "Strata";
		$descr->descr = "Strata data";
		$descr->dbUploadDirKey = "processor.l4a.reference_data_dir";
		$descr->uploadRelPath = "strata";
		$descr->expectedUploadFileExt = ".zip";
		$descr->fileExt = "shp";
		$uploadFileDescriptorArray[] = $descr;
	} else {
		$descr = new UploadFileDescriptor();
		$descr->id = "Lpis";
		$descr->descr = "Declarations";
		$descr->dbUploadDirKey = "processor.lpis.upload_path";
		$descr->uploadRelPath = "";
		$descr->expectedUploadFileExt = ".zip";
		$descr->fileExt = "shp";
		$descr->addParams = '[
						{"id":"year",
						 "label":"Year:",
						 "type":"text",
						 "required":0
						},
						{"id":"importMode",
						 "label":"Method:",
						 "type":"select",
						 "required":1,
						 "options" : [{"id":"UPDATE", "label":"Update existing LPIS", "value" : "UPDATE"},
									  {"id":"REPLACE", "label":"Replace existing LPIS", "value" : "REPLACE"},
									  {"id":"INCREMENTAL", "label":"Reuse existing LPIS (incremental)", "value" : "INCREMENTAL"}
									 ]
						},
						{"id":"parcelIdCols",
						 "label":"Parcel ID cols:",
						 "type":"text",
						 "required":1
						},
						{"id":"holdingIdCols",
						 "label":"Holding ID cols:",
						 "type":"text",
						 "required":1
						},
						{"id":"cropCodeCols",
						 "label":"Crop code cols:",
						 "type":"text",
						 "required":1,
						 "tooltip":"Something"
						}
					]';
		$uploadFileDescriptorArray[] = $descr;

		$descr = new UploadFileDescriptor();
		$descr->id = "lut";
		$descr->descr = "LUT data";
		$descr->dbUploadDirKey = "processor.lpis.lut_upload_path";
		$descr->uploadRelPath = "";
		$descr->expectedUploadFileExt = ".csv";
		$descr->fileExt = "csv";
		$descr->addParams = '[
						{"id":"year",
						 "label":"Year:",
						 "type":"text",
						 "required":1
						}
					]';
		$uploadFileDescriptorArray[] = $descr;

		$descr = new UploadFileDescriptor();
		$descr->id = "L4bCfg";
		$descr->descr = "L4B configuration";
		$descr->dbUploadDirKey = "processor.s4c_l4b.cfg_upload_dir";
		$descr->uploadRelPath = "";
		$descr->expectedUploadFileExt = ".cfg";
		$descr->fileExt = "cfg";
		$descr->addParams = '[
						{"id":"year",
						 "label":"Year:",
						 "type":"text",
						 "required":1
						},
   						{"id":"mowingStartDate",
						 "label":"Grassland mowing start date:",
						 "type":"text",
						 "required":0,
                         "tooltip":"If not set, the start of the season will be considered"
						}
					]';
		$uploadFileDescriptorArray[] = $descr;

		$descr = new UploadFileDescriptor();
		$descr->id = "L4cCfg";
		$descr->descr = "L4C configuration";
		$descr->dbUploadDirKey = "processor.s4c_l4c.cfg_upload_dir";
		$descr->uploadRelPath = "";
		$descr->expectedUploadFileExt = ".cfg";
		$descr->fileExt = "cfg";
		$descr->addParams = '[
						{"id":"year",
						 "label":"Year:",
						 "type":"text",
						 "required":1
						},
						{"id":"practices",
						 "label":"Practices:",
						 "type":"text",
						 "required":1
						},
						{"id":"country",
						 "label":"Country:",
						 "type":"text",
						 "required":1
						}
					]';
		$uploadFileDescriptorArray[] = $descr;

		$arrPractices = array("CC", "FL", "NFC", "NA");
		foreach ($arrPractices as $practice) {
		     $descr = new UploadFileDescriptor();
		     $descr->id = "L4c" . $practice . "Practices";
		     $descr->descr = "L4C " . $practice . " practices infos";
		     $descr->dbUploadDirKey = "processor.s4c_l4c.ts_input_tables_upload_root_dir";        
		     $descr->uploadRelPath = $practice;                 
		     $descr->expectedUploadFileExt = ".csv";
		     $descr->fileExt = "csv";
		     $descr->addParams = '[
                            {"id":"year", 
                             "label":"Year:", 
                             "type":"text",
                             "required":1
                            }]';        
		     $uploadFileDescriptorArray[] = $descr;
		}
	}
	return $uploadFileDescriptorArray;
}

function getPostUploadCmds() {
	$cmds = array();
	if (ConfigParams::isSen2Agri()) {
		
	} else {
		$cmd = new PostUploadCmd();
		$cmd->isAsyncCmd = 1;
		$cmd->triggerUpDescrIds = array();
		$cmd->triggerUpDescrIds[] = "Lpis";
		$cmd->triggerUpDescrIds[] = "lut";
		$cmd->cmd = "data-preparation.py -s {site_shortname} {param2} {param3} {param4} {param5} {param6} {param7}";
		$cmd->params = '[
							{"id":"param1",
							 "key": "-y {value}",
							 "refUp":"Lpis",
							 "refUpParam":"year",
							 "required":0
							},
							{"id":"param2",
							 "key": "--lpis {value}",
							 "refUp":"Lpis",
							 "refUpParam":"",
							 "required":0
							},
							{"id":"param3",
							 "key": "--lut {value}",
							 "refUp":"lut",
							 "refUpParam":"",
							 "required":0
							},
							{"id":"param4",
							 "key": "--parcel-id-cols {value}",
							 "refUp":"Lpis",
							 "refUpParam":"parcelIdCols",
							 "required":1
							},
							{"id":"param5",
							 "key": "--holding-id-cols {value}",
							 "refUp":"Lpis",
							 "refUpParam":"holdingIdCols",
							 "required":1
							},
							{"id":"param6",
							 "key": "--crop-code-cols {value}",
							 "refUp":"Lpis",
							 "refUpParam":"cropCodeCols",
							 "required":1
							},
							{"id":"param7",
							 "key": "--import-mode {value}",
							 "refUp":"Lpis",
							 "refUpParam":"importMode",
							 "required":1
							}
						]';
		$cmds[] = $cmd;

		$cmd = new PostUploadCmd();
		$cmd->isAsyncCmd = 1;
		$cmd->triggerUpDescrIds = array();
		$cmd->triggerUpDescrIds[] = "L4cCfg";
		$cmd->cmd = "s4c_l4c_import_config.py -s {site_shortname} {yearParam} {countryParam} \"{practicesParam}\" {configFileParam}";
		$cmd->params = '[
                            {"id":"yearParam", 
                             "key": "-y {value}",
                             "refUp":"L4cCfg",
                             "refUpParam":"year",
                             "required":1
                            },
                            {"id":"countryParam", 
                             "key": "-t {value}",
                             "refUp":"L4cCfg",
                             "refUpParam":"country",
                             "required":1
                            },
                            {"id":"practicesParam", 
                             "key": "-p {value}",
                             "refUp":"L4cCfg",
                             "refUpParam":"practices",
                             "required":1
                            },                                                   
                            {"id":"configFileParam", 
                             "key": "-i {value}",
                             "refUp":"L4cCfg",
                             "refUpParam":"",
                             "required":1
                            }                                                   
                        ]';
		$cmds[] = $cmd;
        
		$arrPractices = array("CC", "FL", "NFC", "NA");
		foreach ($arrPractices as $practice) {
		     $cmd = new PostUploadCmd();
		     $cmd->isAsyncCmd = 1;
		     $cmd->triggerUpDescrIds = array();
		     $practiceId = "L4c" . $practice . "Practices";
		     $cmd->triggerUpDescrIds[] = $practiceId;
		     $cmd->cmd = "s4c_l4c_import_practice.py -s {site_shortname} {inputFileParam} {yearParam} -p " . $practice;
		     $cmd->params = '[
                                {"id":"inputFileParam", 
                                 "key": "-i {value}",
                                 "refUp":"' . $practiceId . '",
                                 "refUpParam":"",
                                 "required":1
                                },
                                {"id":"yearParam", 
                                 "key": "-y {value}",
                                 "refUp":"' . $practiceId . '",
                                 "refUpParam":"year",
                                 "required":1
                                }
                            ]';
		     $cmds[] = $cmd;
		}
		// This command will be triggered by any of the uploads
		$arrTriggers = array("Lpis", "lut", "L4cCCPractices", "L4cFLPractices", "L4cNFCPractices", "L4cNAPractices");
		foreach ($arrTriggers as $trigger) {        
		    $cmd = new PostUploadCmd();
		    $cmd->isAsyncCmd = 1;
		    $cmd->triggerUpDescrIds = array();
		    $cmd->triggerUpDescrIds[] = $trigger;
		    $cmd->cmd = "s4c_l4c_export_all_practices.py -s {site_shortname} {yearParam}";
		    $cmd->params = '[
                                {"id":"yearParam", 
                                 "key": "-y {value}",
                                 "refUp":"' . $trigger . '",
                                 "refUpParam":"year",
                                 "required":1
                                }
                            ]';
		    $cmds[] = $cmd;
		}        
	}
	return $cmds;
}

function getUploadDescriptionById($id) {
	$uploadDescrs = getUploadFileDescriptorArray();
	foreach ($uploadDescrs as $uploadDescr) {
		if($uploadDescr->id == $id) {
			return $uploadDescr;
		}
	}
	return null;
}

function getUploadedFile($siteId, $upId, $succUpFiles) {
	// we are actually not searching here for a param but for a file name that was uploaded
	$descr = getUploadDescriptionById($upId);
	$fileName = getValueForJsonKey($succUpFiles, $upId);
	if ($fileName != null) {
		$targetDataDir = getTargetFolder($siteId, $descr);
		return $targetDataDir . DIRECTORY_SEPARATOR . $fileName;
	}
	return null;
}

function getUploadDescrParam($upId, $paramId) {
	$descr = getUploadDescriptionById($upId);
	if ($paramId != "") {
		$paramsJsonObj = json_decode($descr->addParams);
		foreach($paramsJsonObj as $param) {
			if($param->id == $paramId) {
				return $param;
			}
		}
	}
	return null;
}

function getUploadDescriptionFromKey($key, $keyPrefix, $keySuffix) {
	$uploadDescrs = getUploadFileDescriptorArray();
	foreach ($uploadDescrs as $uploadDescr) {
		$genKey = $keyPrefix . $uploadDescr->id . $keySuffix;
		if($genKey == $key) {
			return $uploadDescr;
		}
	}
	return null;
}

function getTiles($siteId,$satelliteId){
	$dbconn = pg_connect(ConfigParams::getConnection()) or die ("Could not connect");
	$rows = pg_query_params($dbconn, "select * from sp_get_site_tiles($1,$2)",array($siteId,$satelliteId)) or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_all_columns($rows): array());
}

function CallRestAPI($method, $url, $data = false) {
	$curl = curl_init();

	switch ($method)
	{
		case "POST":
			curl_setopt($curl, CURLOPT_POST, 1);

			if ($data) {
				curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
				curl_setopt($curl, CURLOPT_HTTPHEADER, array('Content-Type: application/json','Content-Length: ' . strlen($data)));
			}
			break;
		case "DELETE":
			curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "DELETE");

			if ($data) {
				curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
				curl_setopt($curl, CURLOPT_HTTPHEADER, array('Content-Type: application/json','Content-Length: ' . strlen($data)));
			}

			echo "Delete " . $data . "      ";
			break;
		case "PUT":
			curl_setopt($curl, CURLOPT_PUT, 1);
			break;
		default:
			if ($data)
				$url = sprintf("%s?%s", $url, http_build_query($data));
	}

	// Optional Authentication:
	//curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	//curl_setopt($curl, CURLOPT_USERPWD, "username:password");

	curl_setopt($curl, CURLOPT_URL, $url);
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
	//curl_setopt($curl, CURLOPT_TIMEOUT_MS, 600000);   // 10 minutes

	$result = curl_exec($curl);

	curl_close($curl);

	return $result;
}

function endsWith($str, $sub) {
	return (substr ( $str, strlen ( $str ) - strlen ( $sub ) ) === $sub);
}

function getTargetFolder($siteId, $uploadDescr) {
	$dbconn = pg_connect( ConfigParams::getConnection() ) or die ( "Could not connect" );
	if (is_numeric($siteId)) {
		$rows = pg_query($dbconn, "SELECT short_name FROM sp_get_sites() WHERE id = ".$siteId) or die(pg_last_error());
		$siteId = pg_fetch_array($rows, 0)[0];
	}
	$queryStr = "SELECT key, value FROM sp_get_parameters('" . $uploadDescr->dbUploadDirKey ."') WHERE site_id IS NULL";
	$rows = pg_query($dbconn, $queryStr) or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[1];
	
	$targetDataDir = str_replace("{site}", $siteId, $result);
	if (!empty($uploadDescr->uploadRelPath)) {
		$targetDataDir = $targetDataDir . DIRECTORY_SEPARATOR . $uploadDescr->uploadRelPath;
	}
	if (DIRECTORY_SEPARATOR == "\\") {
		$targetDataDir = str_replace("\\", "/", $targetDataDir);
	}
	$targetDataDir = str_replace("//", "/", $targetDataDir);
	
	return $targetDataDir;
}

function getExistingUploadedFileName($siteId, $uploadDescr, $uploadYear) {
	$targetDataDir = getTargetFolder($siteId, $uploadDescr);
	$targetDataDir = str_replace("{year}", $uploadYear, $targetDataDir);
	
	$newest_file = "";
	if (is_dir($targetDataDir)) {
		// echo " IsDir " . "\r\n";
		$files = scandir($targetDataDir, SCANDIR_SORT_DESCENDING);
		$existingFiles = array_diff($files, array('..', '.'));
		foreach($existingFiles as $existingFile) {
			$fullFilePath = $targetDataDir . DIRECTORY_SEPARATOR . $existingFile;
			if (!is_dir($fullFilePath)) {
				$fileExt = pathinfo($fullFilePath, PATHINFO_EXTENSION);
				if (strtolower($fileExt) == $uploadDescr->fileExt) {
					$newest_file = $existingFile;
					break;
				}
			}
		}
	}
	
	return $newest_file;
}

function moveUploadedFilesToFinalDir($srcDir, $destDir) {
	
	if (!file_exists($destDir)) {
		if (!mkdir($destDir, 0777, true)) {
			return false;
		}
	}
	
	// ignore ., ..
	// create a backup folder for the existing files as we don't want to delete them directly
	// TODO: the ignored folders (strata and LUT) should be taken in another manner
	$Ignore = array(".","..","strata", "LUT");
	$existingFiles = array_diff(scandir($destDir), $Ignore);
	if (count($existingFiles) > 0) {
		$date = date_create();
		$formattedDate = date_format($date, "Ymd_His");

		$prevFilesDir = $destDir . DIRECTORY_SEPARATOR . $formattedDate . "_backup_prev_files";
		if (!file_exists($prevFilesDir)) {
			// if we cannot create the folder, we skip backing up
			if (mkdir($prevFilesDir, 0777, true)) {
				foreach($existingFiles as $existingFile) {
					if(!in_array($existingFile,$Ignore) && !is_dir($destDir . DIRECTORY_SEPARATOR . $existingFile)) {
						rename($destDir . DIRECTORY_SEPARATOR . $existingFile, $prevFilesDir . DIRECTORY_SEPARATOR . $existingFile); // rename the file
					}
				}
			}
		}
	}
	// now move actually the files from the src dir to dest dir
	$existingFiles = array_diff(scandir($srcDir), array('..', '.'));
	if (count($existingFiles) > 0) {
		$Ignore = array(".","..");
		foreach($existingFiles as $existingFile) {
			if(!in_array($existingFile,$Ignore)) {
				if (!rename($srcDir . DIRECTORY_SEPARATOR . $existingFile, $destDir . DIRECTORY_SEPARATOR . $existingFile)) { // rename the file
					return false;
				}
			}
		}
	}
	return true;
}

function getValueForJsonKey($jsonKeyValsArr, $key) {
	if ($jsonKeyValsArr != null) {
		foreach ($jsonKeyValsArr as $keyVal) {
			$keyValObj = json_decode($keyVal);
			if ($keyValObj->key == $key) {
				return $keyValObj->value;
			}
		}
	}
	return null;
}

function checkSuccesfullUploadsForCmd($params, $errUpFiles) {
	// check that the upload file params do not appear in the list of error files
	foreach ($params as $param) {
		if ($param->refUpParam === "") {
			if(getValueForJsonKey($errUpFiles, $param->refUp) != null) {
				return false;
			}
		}
	}
	return true;
}

function isCommandTriggered($cmdObj, $succUpFiles) {
	foreach ($cmdObj->triggerUpDescrIds as $triggerId) {
		if(getValueForJsonKey($succUpFiles, $triggerId) != null) {
			return true;
		}
	}
	return false;
}

function buildCommand($cmdObj, $cmdParams, $succUpFiles, $errUpFiles) {
	$cmdStr = $cmdObj->cmd;
	$siteShortName = $_REQUEST ["shortname"];
	
	// First replace the site name in the command
	$cmdStr = str_replace('{site_shortname}', $siteShortName, $cmdStr);
	
	foreach ($cmdParams as $param) {
		// check if the file was successfuly uploaded for the current parameter
		$uploadedFile = getUploadedFile($siteShortName, $param->refUp, $succUpFiles);
		if ($uploadedFile != null) {
			// Get param from the upload descr
			// error_log("Uploader: " . $param->refUp . ", Parameter:  " . $param->refUpParam . ", Uploaded file: " . $uploadedFile);
			if ($param->refUpParam == "") {
				// we have the file name
				$paramVal = $uploadedFile;
			} else {
				$upDescrParam = getUploadDescrParam($param->refUp, $param->refUpParam);
				$postParamId = $param->refUp . $upDescrParam->id;
				$reqParamId = "postUploadCmdParam" . $postParamId;
				// get the needed parameters from _REQUEST
				if (isset ( $_REQUEST [$reqParamId] ) ) {
					$paramVal = $_REQUEST [$reqParamId];
					// error_log("ParamVal : " . $paramVal);
					if ($upDescrParam->type === "select") {
						foreach ($upDescrParam->options as $opt) {
							if ($opt->id === $paramVal) {
								$paramVal = $opt->value;
								break;
							}
						}
					}
				}
			}
			if ($paramVal) {
				$cmdParamKey = $param->key;
				$cmdParamKey = str_replace('{value}', $paramVal, $cmdParamKey);
				$cmdStr = str_replace('{' . $param->id . '}', $cmdParamKey, $cmdStr);
			} else {
				if ($upDescrParam->isRequired == 1) {
					// TODO: give an error here
					return null;
				}
			}
		} else {
			// error_log("The command param " . $param->id . " will be ignored as the file for " . $param->refUp . " was not uploaded!");
			$cmdParamKey = $param->key;
			$cmdParamKey = str_replace('{value}', $paramVal, $cmdParamKey);
			$cmdStr = str_replace('{' . $param->id . '}', '', $cmdStr);
		}
	}
	error_log("Cmd to execute : " . $cmdStr);
	return $cmdStr;
}

function postProcessUploadedFiles() {
	$postUploadCmds = getPostUploadCmds();
	if ($postUploadCmds) {
		if (isset ( $_REQUEST ["succ_up_files"] ) ) {
			$succUpFilesStr = $_REQUEST ["succ_up_files"];
			$succUpFiles = json_decode($succUpFilesStr);
		}
		if (isset ( $_REQUEST ["err_up_files"] ) ) {
			$errUpFilesStr = $_REQUEST ["err_up_files"];
			$errUpFiles = json_decode($errUpFilesStr);
		}
		foreach ($postUploadCmds as $cmdObj) {
			if (!isCommandTriggered($cmdObj, $succUpFiles)) {
				// error_log("cmd not triggered: " . $succUpFilesStr);
				continue;
			}
			$cmdParams = json_decode($cmdObj->params);
			// check that all file parameters were uploaded successfuly in the command
			if (!checkSuccesfullUploadsForCmd($cmdParams, $errUpFiles)) {
				// error_log("no success uploads: " . $succUpFilesStr);
				continue;
			}
			// error_log("building cmd: " . $succUpFilesStr);
			$cmdStr = buildCommand($cmdObj, $cmdParams, $succUpFiles, $errUpFiles);
			if (!$cmdStr) {
				continue;
			}
			// Invoke the command here
			error_log("Executing cmd: " . $cmdStr);
			exec($cmdStr);
		}
	}
}

function get_product_types_from_db() {
	function cmp($a, $b)
	{
		return strcmp($a["description"], $b["description"]);
	}

	$restResult = CallRestAPI("GET", ConfigParams::$REST_SERVICES_URL . "/products/types/");
	$jsonArr = json_decode($restResult, true);
	if (count($jsonArr) > 0) {
		usort($jsonArr, "cmp");
		foreach($jsonArr as $productTypeInfo) {
			$id = $productTypeInfo['id'];
			$short = substr(strtoupper($productTypeInfo['description']), 0, 3);
			?>
			<input class="form-control" id="delete_chk<?=$short?>" type="checkbox" name="delete_chk<?=$short?>" value="<?=$id?>">
			<label class="control-label" for="delete_chk<?=$short?>"><?=$short?></label>
<?php
		}
	}
}

function getCustomUploadFolderRoot($siteId) {
	// get the custom upload root path like: /mnt/upload/siteName/userName_timeStamp/
	$dbconn = pg_connect( ConfigParams::getConnection() ) or die ( "Could not connect" );
	$rows = pg_query($dbconn, "SELECT key, value FROM sp_get_parameters('site.upload_path') WHERE site_id IS NULL") or die(pg_last_error());
	$result = pg_fetch_array($rows, 0)[1];

	//////////////////////////////////////////////////////
	// TODO: Remove this ... is only for testing on windows
	//$result = "c:" . $result;
	//////////////////////////////////////////////////////

	$upload_target_dir = str_replace("{user}", "", $result);

	$result = $siteId;
	if (is_numeric($siteId)) {
		$rows = pg_query($dbconn, "SELECT name FROM sp_get_sites() WHERE id = ".$siteId) or die(pg_last_error());
		$result = pg_fetch_array($rows, 0)[0];
	}
	$root_target_dir = $upload_target_dir . $result;
	
	return $root_target_dir;
}

function createCustomUploadFolder($siteId, $timestamp, $subDir) {
	// create custom upload path like: /mnt/upload/siteName/userName_timeStamp/
	$root_target_dir = getCustomUploadFolderRoot($siteId);
	
	$upload_target_dir = $root_target_dir . DIRECTORY_SEPARATOR . ConfigParams::$USER_NAME . "_" . $timestamp . DIRECTORY_SEPARATOR;
	$upload_target_dir = str_replace("(", "", $upload_target_dir);
	$upload_target_dir = str_replace(")", "", $upload_target_dir);
	$upload_target_dir = str_replace(" ", "_", $upload_target_dir);
	$upload_target_dir = $upload_target_dir . ($subDir != '' ? $subDir . DIRECTORY_SEPARATOR : '');
	if (DIRECTORY_SEPARATOR == "\\") {
		$upload_target_dir = str_replace("\\", "/", $upload_target_dir);
	}
	$upload_target_dir = str_replace("//", "/", $upload_target_dir);
	if (!is_dir($upload_target_dir)) {
		mkdir($upload_target_dir, 0755, true);
	}
	return $upload_target_dir;
}

function preprocessUploadedFile($filename, $uploadTargetDir, $expectedFileExt, $expectedArchFileExt, $chkExtractedShape = true, $source = null) {
	$zip_msg = "";
	$shp_msg = '';
	$shp_file = false;
	if (!endsWith(strtolower($filename), $expectedFileExt)) {
		 $zip_msg = "Your file does not have the expected " . $expectedFileExt . "extension!";
	} else {
		$target_path = $uploadTargetDir . DIRECTORY_SEPARATOR . $filename;
		// if the source is null, then handle directly the target path
		// otherwise, a move is performed (based on the short-circuit capabilities of || )
		if($source == null || move_uploaded_file($source, $target_path)) {
			if (endsWith(strtolower($filename), ".zip")) {
				$zip = new ZipArchive();
				$x = $zip->open($target_path);
				if ($x === true) {
					for ($i = 0; $i < $zip->numFiles; $i++) {
						$filename = $zip->getNameIndex($i);
						if (endsWith(strtolower($filename), $expectedArchFileExt)) {
							$shp_file = $uploadTargetDir . $filename;
							break;
						}
					}
					$zip->extractTo($uploadTargetDir);
					$zip->close();
					unlink($target_path);
					if ($shp_file) {
						$zip_msg = "Your .zip file was uploaded and unpacked successfully!";
					} else {
						$zip_msg = "Your .zip file does not contain any (" . $expectedArchFileExt . ") file!";
					}
				} else {
					$zip_msg = "Your file is not a valid .zip archive!";
				}
			} else {
				// else, nothing to do, the file is just there, just update the $shp_file in case it will be later checked (if configured)
				$shp_file = $target_path;
				// TODO: Handle the eventual other archive types here
			}
			// verify if shape file, if it is the case has valid geometry
			if ($chkExtractedShape && $shp_file) {
				$shape_ok = false;
				exec('scripts/check_shp.py -b ' . $shp_file, $output, $ret);
				if ($ret === FALSE) {
					$shp_msg = 'Invalid command line!';
				} else {
					switch ($ret) {
						case   0: $shape_ok = true; break;
						case   1: $shp_file = false; $shp_msg = 'Unable to open the shape file!'; break;
						case   2: $shp_file = false; $shp_msg = 'Shape file has invalid geometry!'; break;
						case   3: $shp_file = false; $shp_msg = 'Shape file has overlapping polygons!'; break;
						case   4: $shp_file = false; $shp_msg = 'Shape file is too complex!'; break;
						case 127: $shp_file = false; $shp_msg = 'Invalid geometry detection script!'; break;
						default : $shp_file = false; $shp_msg = 'Unexpected error with the geometry detection script!'; break;
					}
				}
				if ($shape_ok) {
					$last_line = $output[count($output) - 1];
					$r = preg_match('/^Union: (.+)$/m', $last_line, $matches);
					if (!$r) {
						$shp_file = false;
						$shp_msg = 'Unable to parse shape!';
					} else {
						$shp_msg = $matches[1];
					}
				}
			}
		} else {
			$zip_msg = "Failed to upload the file you selected!";
		}
	}
	if ($chkExtractedShape && ! $shp_file) {
		$shp_msg = $shp_msg . ' Missing shape file due to a problem with your selected file!';
	}

	return array ( "polygons_file" => $shp_file, "result" => $shp_msg, "message" => $zip_msg );
}

function uploadReferencePolygons($zipFile, $uploadTargetDir, $fileExt, $archivedFileExt, $chkExtractedShape = true) {
	if ($_FILES[$zipFile]["name"]) {
		$filename = $_FILES[$zipFile]["name"];
		$source = $_FILES[$zipFile]["tmp_name"];
		return preprocessUploadedFile($filename, $uploadTargetDir, $fileExt, $archivedFileExt, $chkExtractedShape, $source);
	}
	return array ( "polygons_file" => false, "result" => $chkExtractedShape ? 'Missing shape file due to a problem with your selected file!' : '',
				   "message" => 'Unable to access your selected file!' );
}

function getSatelliteEnableStatus($siteId, $satId) {
	return CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/products/enable/status/" . $satId . "/" . $siteId);
}

function getUploadTargetDir($siteId, $uploadRelPath) {
	$date = date_create();
	$time_stamp = date_timestamp_get($date) + abs(intval(microtime(true)*1000)); // add microtime to prevent having the same temporary upload folder name
	$upload_target_dir = createCustomUploadFolder($siteId, $time_stamp, $uploadRelPath);
	return $upload_target_dir;
}

// Change current year for uploaded files
$uploadYear = date("Y");
$uploadYearChanged = false;
if (isset($_REQUEST['change_upload_year']) && $_REQUEST['change_upload_year'] == 'Change year') {
	$uploadYearChanged = true;
	$uploadYear = (isset($_REQUEST['upload_year']) && $_REQUEST['upload_year'] != "") ? $_REQUEST['upload_year'] : $uploadYear;
	$uploadSiteId = $_REQUEST ['upload_siteid'];
}

// processing add site
if (isset ( $_REQUEST ['add_site'] ) && $_REQUEST ['add_site'] == 'Save New Site') {
	// first character to uppercase.
	$site_name = ucfirst ( $_REQUEST ['sitename'] );
	$site_enabled = "0"; // empty($_REQUEST ['add_enabled']) ? "0" : "1";
	//$l8_enabled = empty($_REQUEST ['chkL8Add']) ? "0" : "1";
	//print_r($_REQUEST);

	# Check if the site already exists
	# TODO: create an endpoint for checking if site with name exists
	$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/sites/");
	if ($restResult == "") {
		die ("Cannot get sites list! Please check that the sen2agri-services are started!");
		$message = "Cannot get sites list!";
		$result =  "Please check that the sen2agri-services are started!";
		$_SESSION['status'] =  "NOK"; $_SESSION['message'] = $message;  $_SESSION['result'] = $result;
		die(Header("Location: {$_SERVER['PHP_SELF']}"));
	} else {
		$jsonArr = json_decode($restResult, true);
		foreach($jsonArr as $siteRetr) {
			$retrSiteName = $siteRetr['name'];
			if ($retrSiteName == $site_name) {
				$message = "Site with name " . $retrSiteName . " already exists!";
				$result =  "Please choose another name!";
				$_SESSION['status'] =  "NOK"; $_SESSION['message'] = $message;  $_SESSION['result'] = $result;
				die(Header("Location: {$_SERVER['PHP_SELF']}"));
			}
		}
	}

	function insertSite($site, $coord, $enbl) {
		$db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
		$sql = "SELECT sp_dashboard_add_site($1,$2,$3)";
		$res = pg_prepare ( $db, "my_query", $sql );
		$res = pg_execute ( $db, "my_query", array (
				$site,
				$coord,
				$enbl
		) ) or die ( "An error occurred." );
		$row = pg_fetch_row($res);
		return $row[0];
	}

//    TODO: In the end, should be used the service to create the site (from the services)
//    For now,
//    $sourceFile = $_FILES["zip_fileAdd"]["tmp_name"];
//    $dataObj = new \stdClass();
//    $dataObj->name = $site_name;
//    $dataObj->zipFilePath = $sourceFile;
//    $dataObj->enabled = false;
//    $jsonObj = json_encode($dataObj);
//    $restResult = CallRestAPI("POST",  ConfigParams::$REST_SERVICES_URL . "/sites/" , $jsonObj);

	// upload polygons
	$uploadTargetDir = getUploadTargetDir($site_name, '');
	$upload          = uploadReferencePolygons("zip_fileAdd", $uploadTargetDir, ".zip", ".shp");
	$polygons_file   = $upload ['polygons_file'];
	$coord_geog      = $upload ['result'];
	$message         = $upload ['message'];
	if ($polygons_file) {
		$site_id = insertSite($site_name, $coord_geog, $site_enabled);
		// update also the L8 enable/disable status
		// $restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/products/" . ($l8_enabled ? "enable":"disable") . "/2/" . $site_id);
		// ask services to refresh the configuration from DB
		//$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/refresh/");
		$_SESSION['status'] =  "OK"; $_SESSION['message'] = "Your site has been successfully added!";
	} else {
		$_SESSION['status'] =  "NOK"; $_SESSION['message'] = $message;  $_SESSION['result'] = $coord_geog;
	}

	// Prevent adding site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}

// processing upload file
if (isset($_REQUEST["upload_file"]) && isset($_REQUEST["site_shortname"])) {
	// error_log(json_encode($_REQUEST), 0);
	$uploadYear = (isset($_REQUEST['upload_year']) && $_REQUEST['upload_year'] != "") ? $_REQUEST['upload_year'] : $uploadYear;
	
	$cleanupTargetDir = true; // Remove old files
	
	$siteShortName   = $_REQUEST ['site_shortname'];
	$upload_descr_id = $_REQUEST ['upload_desc_id'];
	$uploadDescr     = getUploadDescriptionById($upload_descr_id);
	
	$chunk = isset($_REQUEST["chunk"]) ? intval($_REQUEST["chunk"]) : 0;
	$chunks = isset($_REQUEST["chunks"]) ? intval($_REQUEST["chunks"]) : 0;
	if ($chunk == 0 || $chunks == 0) {
		$uploadRelPath   = $uploadDescr->uploadRelPath;
		$uploadTargetDir = getUploadTargetDir($siteShortName, $uploadRelPath);
	} else {
		if(!isset($_REQUEST["upload_target_dir"])) {
			// error_log('{"upload_response" : {"error" : {"code": 100, "message": "Target folder needs to be provided for chunks > 0."}, "id" : "id"}}');
			die('{"upload_response" : {"error" : {"code": 100, "message": "Target folder needs to be provided for chunks > 0."}, "id" : "id"}}');
		}
		$uploadRelPath   = $_REQUEST["upload_rel_path"];
		$uploadTargetDir = $_REQUEST["upload_target_dir"];
	}
	
	if (isset($_REQUEST["name"])) {
		$fileName = $_REQUEST["name"];
	} elseif (!empty($_FILES)) {
		$fileName = $_FILES["file"]["name"];
	} else {
		$fileName = uniqid("file_");
	}
	$filePath = $uploadTargetDir . $fileName;
	
	// Remove old temp files
	if ($cleanupTargetDir) {
		if (!is_dir($uploadTargetDir) || !$dir = opendir($uploadTargetDir)) {
			// error_log('{"upload_response" : {"error" : {"code": 100, "message": "Failed to open temp directory."}, "id" : "id"}}');
			die('{"upload_response" : {"error" : {"code": 100, "message": "Failed to open temp directory."}, "id" : "id"}}');
		}
		while (($file = readdir($dir)) !== false) {
			$tmpfilePath = $uploadTargetDir . $file;
			
			// If temp file is current file proceed to the next
			if ($tmpfilePath == "{$filePath}.part") {
				continue;
			}
			// Remove temp file if it is older than the max age and is not the current file
			if (preg_match('/\.part$/', $file) && (filemtime($tmpfilePath) < time() - $maxFileAge)) {
				@unlink($tmpfilePath);
			}
		}
		closedir($dir);
	}

	// Open temp file
	if (!$out = @fopen("{$filePath}.part", $chunks ? "ab" : "wb")) {
		// error_log('{"upload_response" : {"error" : {"code": 102, "message": "Failed to open output stream."}, "id" : "id"}}');
		die('{"upload_response" : {"error" : {"code": 102, "message": "Failed to open output stream."}, "id" : "id"}}');
	}
	if (!empty($_FILES)) {
		if ($_FILES["file"]["error"] || !is_uploaded_file($_FILES["file"]["tmp_name"])) {
			// error_log('{"upload_response" : {"error" : {"code": 103, "message": "Failed to move uploaded file."}, "id" : "id"}}');
			die('{"upload_response" : {"error" : {"code": 103, "message": "Failed to move uploaded file."}, "id" : "id"}}');
		}
		// Read binary input stream and append it to temp file
		if (!$in = @fopen($_FILES["file"]["tmp_name"], "rb")) {
			// error_log('{"upload_response" : {error" : {"code": 101, "message": "Failed to open input stream."}, "id" : "id"}}');
			die('{""upload_response" : {"error" : {"code": 101, "message": "Failed to open input stream."}, "id" : "id"}}');
		}
	} else {
		if (!$in = @fopen("php://input", "rb")) {
			// error_log('{"upload_response" : {"error" : {"code": 101, "message": "Failed to open input stream."}, "id" : "id"}}');
			die('{""upload_response" : {"error" : {"code": 101, "message": "Failed to open input stream."}, "id" : "id"}}');
		}
	}
	while ($buff = fread($in, 4096)) {
		fwrite($out, $buff);
	}
	@fclose($out);
	@fclose($in);
	// Check if file has been uploaded
	if (!$chunks || $chunk == $chunks - 1) {
		// Strip the temp .part suffix off
		// error_log("renaming {$filePath}.part to $filePath");
		rename("{$filePath}.part", $filePath);
		
		// Handle now the uploaded file
		$upload        = preprocessUploadedFile($fileName, $uploadTargetDir, $uploadDescr->expectedUploadFileExt, $uploadDescr->fileExt, false, null);
		$polygons_file = $upload ['polygons_file'];
		$validationMsg = $upload ['result'];
		$uploadMsg     = $upload ['message'];
		if ($polygons_file) {
			$targetFolder = getTargetFolder($siteShortName, $uploadDescr);
			$targetFolder = str_replace("{year}", $uploadYear, $targetFolder);
			$resultMoveFiles = moveUploadedFilesToFinalDir($uploadTargetDir, $targetFolder);
			
			//error_log($uploadTargetDir . " -> " . $targetFolder);
			if (!$resultMoveFiles) {
				die('{"upload_response" : {"error" : {"code": 103, "message": "Cannot move ' . strtolower($uploadDescr->id) . '  files from ' . $uploadTargetDir . ' to ' . $targetFolder . '"}, "id" : "id"}}');
			} else {
				$delDir = $uploadTargetDir;
				if ($uploadRelPath != "") {
					$pos = strrpos($uploadTargetDir, $uploadRelPath);
					if ($pos !== false) {
						$delDir = substr($delDir, 0, $pos);
					}
				}
				
				//removeTempDir($uploadTargetDir, $uploadRelPath);
				die('{"upload_response" : {"message" : "' . $uploadDescr->id . ' file successfuly uploaded!", "id" : "' . $uploadDescr->id . '", "upload_target_dir" : "' . $uploadTargetDir . '", "upload_relevant_file_path" : "' . basename($polygons_file) . '"}}');
			}
		} else {
			$errMsg = $upload ['message'];
			if ($validationMsg != '') {
				$errMsg = $validationMsg;
			}
			// error_log('{"upload_response" : {"error" : {"code": 103, "message": "Error uploading ' . strtolower($uploadDescr->id) . ' file. Error was: ' . $errMsg . '"}, "id" : "id"}}');
			die('{"upload_response" : {"error" : {"code": 103, "message": "Error preprocessing uploaded file ' . $fileName . ' for ' . strtolower($uploadDescr->id) . '. Error was: ' . $errMsg . '"}, "id" : "id"}}');
		}
	}
	// Return Success JSON-RPC response
	die('{"upload_response" : {"result" : {"message" : "success"}, "id" : "' . $uploadDescr->id . '", "upload_target_dir" : "' . $uploadTargetDir . '", "upload_rel_path" : "' . $uploadRelPath . '"}}');
}

// processing LPIS/LUT Import
$arrStartImportTokens = array("LPIS", "L4bCfg", "L4cCfg", "CC", "FL", "NFC", "NA");
$startImportSet = 0;

error_log(json_encode($_REQUEST), 0);

foreach ($arrStartImportTokens as $startImportToken) {
    $lcStartImportToken = strtolower ($startImportToken);
    if (isset($_REQUEST[$lcStartImportToken . '_start_import']) && $_REQUEST[$lcStartImportToken . '_start_import'] == $startImportToken . ' Start Import') {
        $startImportSet = 1;
        break;
    }
}

// error_log("startImportSet is : " . strval($startImportSet), 0);

if ($startImportSet == 1){
    $globalRestResult = '';
	if (isset($_REQUEST['lpis_start_import']) && $_REQUEST['lpis_start_import'] == 'LPIS Start Import') {
        $lpisUrl = ConfigParams::$REST_SERVICES_URL . "/auxdata/import/lpis?siteId=" . $_REQUEST['importSiteId'] . "&year=" . $_REQUEST['importYear'];
        // add Lpis parameters
        if (isset($_REQUEST['lpis_on']) && $_REQUEST['lpis_on'] == "1") {
            $lpisUrl .= "&mode="           . $_REQUEST['mode'];
            $lpisUrl .= "&parcelColumns="  . $_REQUEST['parcelColumns'];
            $lpisUrl .= "&holdingColumns=" . $_REQUEST['holdingColumns'];
            $lpisUrl .= "&cropCodeColumn=" . $_REQUEST['cropCodeColumn'];
            $lpisUrl .= "&lpisFile="       . $_REQUEST['lpisFile'];
        }
        // add Lut parameters
        if (isset($_REQUEST['lut_on']) && $_REQUEST['lut_on'] == "1") {
            $lpisUrl .= "&lutFile=" . $_REQUEST['lutFile'];
        }
        
        error_log($lpisUrl, 0);
        
        //CallRestAPI($method, $url, $data = false)
        $restResult = CallRestAPI("GET", $lpisUrl);
        $globalRestResult = $restResult;
    }
    if (isset($_REQUEST['l4bcfg_start_import']) && $_REQUEST['l4bcfg_start_import'] == 'L4bCfg Start Import'){
        $l4bCfgUrl = ConfigParams::$REST_SERVICES_URL . "/auxdata/import/l4bcfg?siteId=" . $_REQUEST['importSiteId'] . "&year=" . $_REQUEST['importYear'];
        $l4bCfgUrl .= "&l4bCfgFile="       . $_REQUEST['l4bCfgFile'];
        if (isset($_REQUEST['mowingStartDate']) && !empty($_REQUEST['mowingStartDate'])) {
            $l4bCfgUrl .= "&mowingStartDate="  . $_REQUEST['mowingStartDate'];
        }
        error_log($l4bCfgUrl, 0);
        //CallRestAPI($method, $url, $data = false)
        $restResult = CallRestAPI("GET", $l4bCfgUrl);
        $globalRestResult = $globalRestResult . "S4C L4B Import Result : " . $restResult;
    }
    
    if (isset($_REQUEST['l4ccfg_start_import']) && $_REQUEST['l4ccfg_start_import'] == 'L4cCfg Start Import'){
        $l4cCfgUrl = ConfigParams::$REST_SERVICES_URL . "/auxdata/import/l4ccfg?siteId=" . $_REQUEST['importSiteId'] . "&year=" . $_REQUEST['importYear'];
        $l4cCfgUrl .= "&practices="       . $_REQUEST['l4cCfgpractices'];
        $l4cCfgUrl .= "&country="       . $_REQUEST['l4cCfgCountry'];
        $l4cCfgUrl .= "&l4cCfgFile="    . $_REQUEST['l4cCfgFile'];
        
        error_log($l4cCfgUrl, 0);
        //CallRestAPI($method, $url, $data = false)
        $restResult = CallRestAPI("GET", $l4cCfgUrl);
        $globalRestResult = $globalRestResult . "S4C L4B Import Result : " . $restResult;
    }
    $arrPractices = array("CC", "FL", "NFC", "NA");
    foreach ($arrPractices as $practice) {
        $lcPractice = strtolower ($practice);
        if (isset($_REQUEST[$lcPractice . '_start_import']) && $_REQUEST[$lcPractice . '_start_import'] == $practice . ' Start Import') {
            $practiceCfgUrl = ConfigParams::$REST_SERVICES_URL . "/auxdata/import/l4cpractice?siteId=" . $_REQUEST['importSiteId'] . "&year=" . $_REQUEST['importYear'];
            $practiceCfgUrl .= "&practice="        . $practice;
            $practiceCfgUrl .= "&practiceFile="    . $_REQUEST["l4c" . $practice . "PracticesFile"];
            error_log($practiceCfgUrl, 0);
            //CallRestAPI($method, $url, $data = false)
            $restResult = CallRestAPI("GET", $practiceCfgUrl);
            $globalRestResult = $globalRestResult . "S4C " . $practice . " Practice Import Result : " . $restResult;
        }
    }
    
	$restResult = str_replace("'", "`", $restResult);
	
	// Return Success JSON-RPC response
	if (strpos($restResult, "{") < 0) {
		$restResult = '{"data": "error", "message": "' . str_replace('"', '`', $restResult) . '"}';
	}
	die('all_import_done_response->' . $restResult);
}

// processing edit site
if (isset ( $_REQUEST ['edit_site'] ) && $_REQUEST ['edit_site'] == 'Save Site') {
	// error_log(json_encode($_REQUEST), 0);
	
	$site_id      = $_REQUEST ['edit_siteid'];
	$shortname    = $_REQUEST ['shortname'];
	$site_enabled = empty($_REQUEST ['edit_enabled']) ? "0" : "1";
	$l8_enabled   = empty($_REQUEST ['chkL8Edit']) ? "0" : "1";
	$tilesS2      = isset($_REQUEST ['S2Tiles']) ? $_REQUEST ['S2Tiles']:'';
	  
	if($site_enabled){
		if($l8_enabled){
			$tilesL8 = isset($_REQUEST ['L8Tiles']) ? $_REQUEST ['L8Tiles']:'';
	
			$tilesL8 = explode(',', trim($tilesL8));
			$match = array();
			if(sizeof($tilesL8)>0){
				// get available tiles for L8
				$availableTiles = getTiles($site_id,2);
				foreach ($tilesL8 as $tileL8){
					if(preg_match('/^(\d{6})(,\d{6})*/', $tileL8, $matches)){
						if(in_array($matches[0], $availableTiles)){
							$match[]= $matches[0];
						}
					}
				}
				if(sizeof($match)>0){
					exec('filter_site_download_tiles.py -t 2 -s '.$shortname.' -e '.$site_enabled.' -l "'.implode(", ", $match).'"',$output, $ret);
				}
			}
		}
	   
		//insert site tiles for satellite S2
		$tilesS2 = explode(',', trim($tilesS2));
		$matchS2 = array();
		if(sizeof($tilesS2)>0){
		// get available tiles for S2
			$availableTiles = getTiles($site_id,1);
			foreach ($tilesS2 as $tileS2){
				if(preg_match('/^(\d{2}[A-Z]{3})(,\d{2}[A-Z]{3})*/', $tileS2, $matches) ){
					if(in_array($matches[0], $availableTiles)){
						$matchS2[]= $matches[0];
					}
				}
			}
		
			if(sizeof($matchS2)>0){
				exec('filter_site_download_tiles.py -t 1 -s '.$shortname.' -e '.$site_enabled.' -l "'.implode(", ", $matchS2).'"',$output, $ret);
			}
		}
	}
	
	function uploadFileSelected($name) {
		foreach($_FILES as $key => $val){
			if (($key == $name) && (strlen($_FILES[$key]['name'])) > 0) {
				return true;
			}
		}
		return false;
	}

	function updateSite($id, $enbl) {
		$db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
		$res = pg_query_params ( $db, "SELECT sp_dashboard_update_site($1,$2)", array (
				$id,
				$enbl
		) ) or die ( "An error occurred." );
	}

	// upload polygons if zip file selected
	$status  = "OK";
	$message = "";
	
	if ($status == "OK") {
		updateSite($site_id, $site_enabled);
		
		// update also the L8 enable/disable status
		$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/products/" . ($l8_enabled == "1" ? "enable":"disable") . "/2/" . $site_id);
		// stop or enable the satellite for the L8 satellite or for both satellites if the site is disabled/enabled
		if ($site_enabled == "1") {
			$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/downloader/start/" . $site_id . "/1");
			if ($l8_enabled == "1") {
				$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/downloader/start/" . $site_id . "/2");
			} else {
				$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/downloader/stop/" . $site_id . "/2");
			}
		} else {
			$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/downloader/stop/" . $site_id . "/1");
			$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/downloader/stop/" . $site_id . "/2");
		}
		
		// Refresh the configuration
		//$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/refresh/");
		
		$message = "Your site has been successfully modified!";
	}
	
	/*
	$allMsgs = array();
	$uploadDescrs = getUploadFileDescriptorArray();
	foreach ($uploadDescrs as $uploadDescr) {
		$uploadKey = "site" . $uploadDescr->id . "DataUpload";
		if (uploadFileSelected($uploadKey)) {
			$uploadTargetDir = getUploadTargetDir($shortname, $uploadDescr->uploadRelPath);
			$upload        = uploadReferencePolygons($uploadKey, $uploadTargetDir, $uploadDescr->expectedUploadFileExt, $uploadDescr->fileExt, false);
			$polygons_file = $upload ['polygons_file'];
			$validationMsg = $upload ['result'];
			$uploadMsg     = $upload ['message'];
			if ($polygons_file) {
				$targetFolder = getTargetFolder($shortname, $uploadDescr);
				$resultMoveFiles = moveUploadedFilesToFinalDir($uploadTargetDir, $targetFolder);
				if (!$resultMoveFiles) {
					$uploadMsg ="Cannot move " . strtolower($uploadDescr->id) . " files from " . $uploadTargetDir . " to " . $targetFolder . "\\n";
				} else {
					$uploadMsg = $uploadDescr->id . " file successfuly uploaded!\\n";
				}
			} else {
				$errMsg = $upload ['message'];
				if ($validationMsg != '') {
					$errMsg = $validationMsg;
				}
				$uploadMsg = "Error uploading " . strtolower($uploadDescr->id) . " file. Error was: " . $errMsg . "\\n";
			}
			$allMsgs[] = $uploadMsg;
		}
	}
	
	// execute the commands for successfuly uploaded files
	postProcessUploadedFiles();
	
	if (isset($_REQUEST ['upload_msgs']) && ($_REQUEST ['upload_msgs'] != "")) {
		$allMsgs[] = json_encode($_REQUEST ['upload_msgs']);
	}
	
	if (count($allMsgs) > 0) {
		$nonEmptyMsgs = 0;
		foreach ($allMsgs as $msg) {
			if (!empty($msg)) {
				if ($nonEmptyMsgs == 0) {
					$message = $message . "\\nAlso:\\n";
				}
				$message = $message . json_encode($msg);
				$nonEmptyMsgs++;
			}
		}
	}
	*/
	
	$_SESSION['status'] =  $status; $_SESSION['message'] = $message;

	// Prevent updating site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}

// processing  delete_site
if (isset ( $_REQUEST ['delete_site_confirm'] ) && $_REQUEST ['delete_site_confirm'] == 'Confirm Delete Site') {
	$shortname      = $_REQUEST ['delete_site_short_name'];

	$arr_result = array();
	foreach($_POST as $key => $value) {
		if (strpos($key, 'delete_chk') === 0) {
			// value starts with delete_chk
			$arr_result[] = (int)$value;
		}
	}
	$dataObj = new \stdClass();
	$dataObj->siteId = -1;
	$dataObj->siteShortName = $shortname;
	$dataObj->productTypeIds = $arr_result;
	$jsonObj = json_encode($dataObj);

	$restResult = CallRestAPI("DELETE",  ConfigParams::$REST_SERVICES_URL . "/sites/" , $jsonObj);

	$status  = "OK";
	$message = "";

	if ($status == "OK") {
		//updateSite($site_id, $site_enabled);
		$message = "Your site " . $shortname . " has been successfully removed!";
	}
	$_SESSION['status'] =  $status; $_SESSION['message'] = $message;

	// Prevent updating site when refreshing page
	die(Header("Location: {$_SERVER['PHP_SELF']}"));
}

?>
<style>
	div.form-group.required{position:relative;float:left}
	div.panel.uploading div.form-group.required.invalid::after{
		content: "*";
		color: white;
		font-size: 24px;
		line-height: 1.1em;
		text-align: right;
		position: absolute;
		top: 6px;
		right: 1px;
		padding: 0 3px 0 0;
		width: 25px;
		height: 25px;
		background: linear-gradient(to top right, transparent 0%, transparent 50%, #ff000090 50%, #ff000090 100%);
		border-radius: 0 2px 0 0;
		pointer-events: none;
	}
	div.container-fluid .icon {
		display: none;
		border: 1px solid transparent;
		border-radius: 5px;
		width: 22px;
		height: 18px;
		padding: 0;
		background-position: 50% 50%;
		background-repeat: no-repeat;
		background-color: transparent;
		background-image: url(../images/cancel.png);
		position: absolute;
		right: 2px;
		top: 6px;
	}
	div.container-fluid .uploading .icon {
		display: block;
	}
	div.container-fluid .moxie-shim{display:none}
	input.upload-file{
		padding: 5px 10px 5px 110px;
		height: 22px;
		width: 200px;
		border: 1px solid #000;
		border-radius: 3px;
	}
	.ui-dialog{height:auto!important}
	input.left-btn{float:left;margin-left:0!important;margin-right:15px!important}
	input.delete-btn{background-color:#d42e11}
	input.delete-btn:not(:disabled):hover{background-color:#e72f0f}
	input.upload-btn{background-color:#487bd4}
	input.upload-btn:not(:disabled):hover{background-color:#578ff1}
	input.cancel-btn{background-color:gray}
	input.cancel-btn:not(:disabled):hover{background-color:#949494}
	.container-fluid{padding:0}
</style>
<div id="main">
	<div id="main2">
		<div id="main3">
			<!-- Start code for adding site---------- -->
			<?php
			?>
			<div class="panel panel-default create-site">
				<div class="panel-body">
					<?php
					if (!$_SESSION['isAdmin']) {
						// not admin ?>
						<div class="panel-heading row">Seasons site details</div>
					<?php } else {
						// admin ?>
						<div class="panel-heading row"><input name="addsite" type="button" class="add-edit-btn" value="Create new site" onclick="formAddSite()" style="width: 200px"></div>
					<?php } ?>
					<!---------------------------  form  add site ------------------------>
					<div class="add-edit-site" id="div_addsite" style="display: none;">
						<form enctype="multipart/form-data" id="siteform" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-1">
									<div class="form-group  form-group-sm">
										<label class="control-label" for="sitename">Site name:</label>
										<input type="text" class="form-control" id="sitename" name="sitename">
									</div>
<!--
									<div class="form-group form-group-sm sensor">
												<label  style="">Enabled sensor:</label>
												<input class="form-control chkS2" id="chkS2Add" type="checkbox" name="chkS2Add" value="S2" checked="checked" disabled>
												<label class="control-label" for="lchkS2Add">S2</label>
												<input class="form-control chkL8" id="chkL8Add" type="checkbox" name="chkL8Add" value="L8" checked="checked">
												<label class="control-label" for="chkL8Add">L8</label>
									</div>
-->
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" style="color:gray">Seasons:</label>
										<h6 style="color:gray;padding:0px 10px 10px 10px;">Seasons can only be added/modified after site creation</h6>
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1">
									<div class="form-group form-group-sm">
										<label class="control-label" for="zip_fileAdd">Upload site shape file:</label>
										<input type="file" class="form-control" id="zip_fileAdd" name="zip_fileAdd">
									</div>
								</div>
							</div>
							<div class="submit-buttons">
								<input class="add-edit-btn" name="add_site" type="submit" value="Save New Site">
								<input class="add-edit-btn cancel-btn" name="abort_add" type="button" value="Abort" onclick="abortEditAdd('add')">
							</div>
						</form>
					</div>
					<!---------------------------- end form add ---------------------------------->

					<!---------------------------  form upload files ------------------------>
					<div class="add-edit-site" id="div_uploadfiles" style="display:none;">
						<form enctype="multipart/form-data" id="siteform_upload" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-5">
									<div class="form-group form-group-sm">
										<label class="control-label" for="upload_sitename">Site name:</label>
										<input type="text" class="form-control" id="upload_sitename" name="upload_sitename" value="" readonly>
										<input type="hidden" id="upload_siteid" name="upload_siteid" value="">
										<input type="hidden" id="succ_up_files" name="succ_up_files" value="">
										<input type="hidden" id="err_up_files" name="err_up_files" value="">
										<input type="hidden" id="upload_msgs" name="upload_msgs" value=""/>
									</div>
								</div>
								<div class="col-md-5">
									<div class="form-group form-group-sm">
										<label class="control-label" for="upload_year" style="width:100%">Configuration year:</label>
										<input type="text" class="form-control" id="upload_year" name="upload_year" value="<?= $uploadYear ?>" style="width:100px;float:left;margin-right:10px">
										<input type="submit" class="form-control add-edit-btn upload-btn left-btn" id="change_upload_year" name="change_upload_year" value="Change year" style="width:auto">
										<input type="hidden" name="upload_msgs" id="upload_msgs" value=""/>
									</div>
								</div>
							</div>
						</form>
						<div class="container-fluid" style="font:14px Arial, Helvetica, sans-serif">
							<div class="panel-group config" id="accordion">
								<?php
								$uploadDescrs = getUploadFileDescriptorArray();
								foreach ($uploadDescrs as $uploadDescr) {
								?>
									<div class="panel panel-default" id="panel<?= $uploadDescr->id ?>" data-id="<?= $uploadDescr->id ?>">
										<div class="panel-heading">
											<h4 class="panel-title">
												<a data-toggle="collapse" data-parent="#accordion" href="#site<?php echo $uploadDescr->id ?>DataAcc"
												   id="site<?php echo $uploadDescr->id ?>DataAccHref"><?php echo $uploadDescr->descr ?></a>
											</h4>
										</div>
										<div id="site<?php echo $uploadDescr->id ?>DataAcc" class="panel-collapse collapse">
											<div class="panel-body">
												<?php
												if ($uploadDescr->addParams) {
													$paramsJson = json_decode($uploadDescr->addParams);
													foreach ($paramsJson as $param) {
														$paramId = $uploadDescr->id . $param->id;
														?>
														<div class="form-group form-group-sm<?= ($param->required==1 ? ' required' : '') ?>">
															<label class="inputlabel" for="postUploadCmdParam<?php echo $paramId ?>"><?php echo $param->label?> </label>
															<?php if ($param->type === "select") { ?>
																<select class="form-control labelinput" name="postUploadCmdParam<?php echo $paramId ?>" id="postUploadCmdParam<?php echo $paramId ?>"<?= ($param->required==1 ? ' required' : '') ?>>
																	<?php foreach ($param->options as $opt) { ?>
																		 <option value="<?php echo $opt->id ?>"><?php echo $opt->label ?></option>
																	<?php } ?>
																</select>
															<?php } else if ($param->type === "text") { ?>
																<input type="text" class="form-control labelinput" name="postUploadCmdParam<?php echo $paramId ?>" id="postUploadCmdParam<?php echo $paramId ?>"
																		<?= ($param->id=='year'  ? ' value="' . $uploadYear .'"' : '') ?>
																		<?= ($param->id=='year'  ? ' readonly' : '') ?>
																		<?= ($param->required==1 ? ' required' : '') ?>
																/>
															<?php } ?>
														</div>
														<?php
													}
												} ?>
												<div class="form-group form-group-sm">
													<label class="inputlabel" for="site<?php echo $uploadDescr->id ?>Data">Existing file:</label>
													<input type="text" class="form-control labelinput" id="site<?php echo $uploadDescr->id ?>Data" name="site<?php echo $uploadDescr->id ?>Data" value="" readonly>
												</div>
												<div class="form-group form-group-sm">
													<label class="inputlabel" for="container<?php echo $uploadDescr->id ?>">Upload file:</label>
													<div id="container<?php echo $uploadDescr->id ?>" style="display:inline-block">
														<input type="text" id="filelist<?= $uploadDescr->id ?>" name="filelist<?= $uploadDescr->id ?>" onchange="onNewFileSelected('<?= $uploadDescr->id ?>', 'site<?= $uploadDescr->id ?>DataAccHref');"
															placeholder="No file selected." class="upload-file" value="" readonly />
														 <div style="position:absolute;top:3px;left:3px">
															<input type="button" id="pickfiles<?php echo $uploadDescr->id ?>" value=" Browse ... " style="padding:5px 10px 4px 10px;border-radius:3px;border:1px solid grey" />
														</div>
														 <input type="button" name="remove" title="Remove file" class="icon" onclick="onRemoveFile('<?= $uploadDescr->id ?>', 'site<?= $uploadDescr->id ?>DataAccHref');" />
													</div>
												</div>
											</div>
										</div>
									</div>
								<?php } ?>
							</div>
							<div class="submit-buttons" style="margin-bottom:15px">
								<input class="add-edit-btn upload-btn left-btn" name="start_upload_files" id="start_upload_files" type="button" value="Start Upload">
								<input class="add-edit-btn cancel-btn" name="abort_add" type="button" value="Close" onclick="abortEditAdd('upload_files')">
							</div>
						</div>
					</div>
					<!---------------------------- end form Upload ---------------------------------->
					
					<!---------------------------  form  delete site ------------------------>
					<div class="add-edit-site" id="div_deletesite" style="display: none;">
						<form enctype="multipart/form-data" id="siteform_delete" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-11">
									<div class="form-group  form-group-sm">
										<label class="control-label" for="delete_sitename">Site name: </label>
										<input type="text" class="form-control" id="delete_sitename" name="delete_sitename" value="" readonly>
										<input type="hidden" class="form-control" id="delete_site_short_name" name="delete_site_short_name" value="">
									</div>
								</div>
							</div>
							<div class="form-group form-group-sm sensor" id="delete_checkboxes_div">
								<label  style="">To delete:</label>
								<?php get_product_types_from_db(); ?>
							</div>
							<div class="submit-buttons">
								<input class="add-edit-btn delete-btn left-btn" name="delete_site_confirm" type="submit" value="Confirm Delete Site">
								<input class="add-edit-btn cancel-btn" name="abort_add" type="button" value="Abort" onclick="abortEditAdd('delete_site')">
							</div>
						</form>
					</div>
					<!---------------------------- end form Delete ---------------------------------->

					<!---------------------------- form edit sites ------------------------------->
					<div class="add-edit-site" id="div_editsite" style="display: none;">
						<!--   onsubmit="return doUploadChangedFiles()" -->
						<form enctype="multipart/form-data" id="siteform_edit" action="create_site.php" method="post">
							<div class="row">
								<div class="col-md-11">
									<div class="form-group form-group-sm">
										<label class="control-label" for="edit_sitename">Site name:</label>
										<input type="text" class="form-control" id="edit_sitename" name="edit_sitename" value="" readonly>
										<input type="hidden" id="edit_siteid" name="edit_siteid" value="">
										<input type="hidden" id="shortname" name="shortname" value="">
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-1" style="width:100px;padding-right:0;padding-left:30px" >
									<label style="">Enabled sensors:</label>
								</div>
								<div class="col-md-9">
									<div class="row">
										<div class="col-md-1" style="width:10%;padding-right:0px">
											<div class="form-group form-group-sm">
												<input class="chkS2" id="chkS2Edit" type="checkbox" name="chkS2Edit" value="S2" checked="checked" disabled ">
												<label class="control-label" for="chkS2Edit">S2</label>
											</div>
										</div>
										<div class="col-md-9">
											<div class="form-group form-group-sm">
												<textarea class='form-control' style="resize: vertical;" rows="3" name="S2Tiles"></textarea>
												<span class="invalidTilesS2"></span>
											</div>
										</div>
									</div>
									<div class="row">
										<div class="col-md-1" style="width: 10%;padding-right: 0px;">
											<div class="form-group form-group-sm">
												<input class="chkL8" id="chkL8Edit" type="checkbox" name="chkL8Edit" value="L8" checked="checked" onchange="enableDisable($(this), $('[name=\'edit_enabled\']').bootstrapSwitch('state'))">
												<label  for="chkL8Edit">L8</label>
											</div>
										</div>
										<div class="col-md-9" >
									   		<div class="form-group form-group-sm">
												<textarea class='form-control' style="resize: vertical;" rows="3" name="L8Tiles""></textarea>
												<span class="invalidTilesL8"></span>
											</div>
										</div>
									</div>
							  	</div>
							</div>
							<div class="row">
								<div class="col-md-11">
									<div class="form-group form-group-sm">
										<label class="control-label">List of Seasons</label>
										<div id="site-seasons"><img src="./images/loader.gif" width="64px" height="64px"></div>
									</div>
								</div>
							</div>
							<div class="row">
								<div class="col-md-11">
									<div class="form-group form-group-sm">
										<label class="control-label" for="edit_enabled">Enable site:</label>
										<input type="checkbox" name="edit_enabled" id="edit_enabled">
									</div>
								</div>
							</div>
							<div class="submit-buttons">
								<input class="add-edit-btn delete-btn left-btn" name="delete_site" type="button" value="Delete Site" onclick="formDeleteSite()">
								<input class="add-edit-btn upload-btn" name="upload_files" type="button" value="Upload Files" onclick="formUploadFiles()">
								<input class="add-edit-btn" name="edit_site" id="edit_site_submit_btn" type="submit" value="Save Site">
								<!--input class="add-edit-btn cancel-btn" name="abort_edit" type="button" value="Abort" onclick="abortEditAdd('edit')"-->
							</div>
						</form>
					</div>
					<!------------------------------ end form edit sites -------------------------------->

					<!------------------------------ list of sites -------------------------------------->
					<table class="table table-striped">
						<thead>
							<tr>
								<th rowspan="2">Site name</th>
								<th rowspan="2">Short name</th>
								<th class="seasons">Seasons</th>
								<th rowspan="2">Edit</th>
								<th rowspan="2">Enabled</th>
							</tr>
							<tr>
								<th>
									<table class="subtable"><thead><th>Season name</th><th>Season start</th><th>Season mid</th><th>Season end</th><th>Enabled</th></thead><tbody></tbody></table>
								</th>
							</tr>
						</thead>
						<tbody>
						<?php

						$restResult = CallRestAPI("GET",  ConfigParams::$REST_SERVICES_URL . "/sites/");
						 if ($restResult == "") {
							print_r ("Cannot get sites list! Please check that the sen2agri-services are started!");
						} else {
							$jsonArr = json_decode($restResult, true);
							$userSites = (isset($_SESSION['siteId']) && sizeof($_SESSION['siteId'])>0)? $_SESSION['siteId']:array();
							if(is_array($jsonArr)){
								$uploadDescrs = getUploadFileDescriptorArray();
								foreach($jsonArr as $site) {
								if((sizeof($userSites)>0 && in_array($site['id'], $userSites)) || (sizeof($userSites) == 0 && $_SESSION['isAdmin'])){
									$siteId       = $site['id'];
									$siteName     = $site['name'];
									$shortName    = $site['shortName'];
									$site_enabled = $site['enabled'];
									$existingUploadedFiles = array();
									foreach ($uploadDescrs as $uploadDescr) {
										$existingUploadedFiles[] = getExistingUploadedFileName($shortName, $uploadDescr, $uploadYear);
									}
									$siteL8Enabled = (getSatelliteEnableStatus($siteId, 2) == "false" ? "" : "checked");  // only L8 for now
									?>
									<tr data-id="<?= $siteId ?>">
										<td><?= $siteName ?></td>
										<td><?= $shortName ?></td>
										<td class="seasons"></td>
										<td class="link"><a onclick='formEditSite(<?= $siteId ?>,"<?= $siteName ?>","<?= $shortName ?>",<?= $site_enabled ? "true" : "false" ?>,
										<?= json_encode($existingUploadedFiles) ?>,  "<?= $siteL8Enabled ?>")'>Edit</a></td>
										<td><input type="checkbox" name="enabled-checkbox"<?= $site_enabled ? "checked" : "" ?>></td>
									</tr>
						<?php  } } } } ?>
						</tbody>
					</table>
					<!------------------------------ end list sites ------------------------------>
				</div>
			</div>
			<!-- End code for adding site---------- -->
		</div>
	</div>
</div>

<!-- includes for datepicker -->
<link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
<script src="libraries/jquery-ui/jquery-ui.min.js"></script>

<!-- includes for bootstrap-switch -->
<link rel="stylesheet" href="libraries/bootstrap-switch/bootstrap-switch.min.css">
<script src="libraries/bootstrap-switch/bootstrap-switch.min.js"></script>

<!-- includes for validate form -->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>
<script src="libraries/plupload-2.3.6/js/plupload.full.min.js"></script>

<script type="text/javascript">
$(document).ready( function() {
	// get all site seasons
	$("table.table td.seasons").each(function() {
		getSiteSeasons($(this).parent().data("id"));
	});

	// create dialog for add site form
	$("#div_addsite").dialog({
		title: "Add New Site",
		width: '560px',
		autoOpen: false,
		modal: true,
		resizable: false,
		beforeClose: function( event, ui ) { resetEditAdd("add"); }
	});

	// create dialog for edit site form
	$("#div_editsite").dialog({
		title: "Edit Site",
		width: '700px',
		autoOpen: false,
		modal: true,
		resizable: false,
		beforeClose: function( event, ui ) { resetEditAdd("edit"); }
	});

	// create dialog for delete site form
	$("#div_deletesite").dialog({
		title: "Delete Site",
		width: '700px',
		autoOpen: false,
		modal: true,
		resizable: false,
		position: { my: "left-4 bottom", at: "left bottom-70", of: "#div_editsite" },
		beforeClose: function( event, ui ) { resetEditAdd("delete"); }
	});

	// create dialog for upload files
	$("#div_uploadfiles").dialog({
		title: "Upload Files",
		width: '700px',
		autoOpen: false,
		modal: true,
		resizable: false,
		beforeClose: function( event, ui ) { resetEditAdd("upload"); }
	});

	// change row style when site editing
	$( ".create-site a" ).click(function() {
		$(this).parent().parent().addClass("editing")
	});

	// create switches for all checkboxes in the sites list
	$("[name='enabled-checkbox']").bootstrapSwitch({
		size: "mini",
		onColor: "success",
		offColor: "default",
		disabled: true,
		handleWidth: 25
	});

	// create switch for enabled checkbox in the add site form
	$("[name='add_enabled']").bootstrapSwitch({
		size: "small",
		onColor: "success",
		offColor: "default"
	});

	// create switch for enabled checkbox in the edit site form
	$("[name='edit_enabled']").bootstrapSwitch({
		size: "small",
		onColor: "success",
		offColor: "default",
		onSwitchChange: function(event,state){
			 enableDisable( $('#chkS2Edit'),state);
			 enableDisable( $('#chkL8Edit'),state);
			}
	});

	// validate add site form
	$("#siteform").validate({
		rules: {
			sitename:{ required: true, pattern: "[A-Z]{1}[\\w ]*" },
			zip_fileAdd: "required"
		},
		messages: {
			sitename: { pattern : "First letter must be uppercase. Letters, digits, spaces and underscores are allowed" }
		},
		highlight: function(element, errorClass) {
			$(element).parent().addClass("has-error");
		},
		unhighlight: function(element, errorClass) {
			$(element).parent().removeClass("has-error");
		},
		errorPlacement: function(error, element) {
			error.appendTo(element.parent());
		},
		submitHandler: function(form) {
			// To POST the new site, use a form's clone in order to prevent cyclic revalidation
			$(document.body).append($(form).clone().attr("name", "siteform_clone").css("display", "none"));
			$("form[name=siteform_clone").submit();
		},
		// set this class to error-labels to indicate valid fields
		success: function(label) {
			label.remove();
		},
	});

	// validate edit site form
	$("#siteform_edit").validate({
		rules: {
			shortname:{ required: true, pattern: "[a-z]{1}[a-z_ ]*" }
		},
		messages: {
			shortname: { pattern : "Only small letters,space and underscore are allowed" }
		},
		highlight: function(element, errorClass) {
			$(element).parent().addClass("has-error");
		},
		unhighlight: function(element, errorClass) {
			$(element).parent().removeClass("has-error");
		},
		errorPlacement: function(error, element) {
			error.appendTo(element.parent());
		},
		submitHandler: function(form) {
			//check the tiles
			var tilesL8NotValid = '';var tilesS2NotValid = '';
			if($("#chkS2Edit").is(':checked')){
				var s2Tiles = $('textarea[name="S2Tiles"]').val();
	
				if(s2Tiles!=""){
					tilesS2Arr = s2Tiles.split(',');
					for(i = 0; i < tilesS2Arr.length; i++){
						var noMatch = tilesS2Arr[i].replace(/^(\d{2}[A-Z]{3})(,\d{2}[A-Z]{3})*/gm,'');
						if(noMatch.trim()!=''){
							tilesS2NotValid = tilesS2NotValid+" "+tilesS2Arr[i];
						}
					}
				}
			}
			if($("#chkL8Edit").is(':checked')){
				var l8Tiles = $('textarea[name="L8Tiles"]').val();
	
				if(l8Tiles!=""){
					tilesL8Arr = l8Tiles.split(',');
					for(i = 0; i < tilesL8Arr.length; i++){
						var noMatch = tilesL8Arr[i].replace(/^(\d{6})(,\d{6})*/gm,'');
						if(noMatch.trim()!=''){
							tilesL8NotValid = tilesL8NotValid+" "+ tilesL8Arr[i];
						}
					}
				}
			}
			if(tilesS2NotValid!=''){
				$(".invalidTilesS2").text("Invalid tile: "+tilesS2NotValid);
			}else{
				$(".invalidTilesS2").text("");
				}
			if(tilesL8NotValid!=''){
				$(".invalidTilesL8").text("Invalid tile: "+tilesL8NotValid);
			}else{
				$(".invalidTilesL8").text("");
			}
			if(tilesS2NotValid=='' && tilesL8NotValid==''){
				// To POST the edited site, use a form's clone in order to prevent cyclic revalidation
				$(document.body).append($(form).clone().attr("name", "siteform_edit_clone").css("display", "none"));
				$("form[name=siteform_edit_clone").submit();
			}
		},
		// set this class to error-labels to indicate valid fields
		success: function(label) {
			label.remove();
		},
	});
	$("div.required").on("change", "input[type='text']", function (e) {
		var panelId = $(this).closest(".uploading").data("id");
		validPanel(panelId);
	});
	
	function uploadStatus(id) {
		var status = { "uploaded": 0, "failed": 0, "total": 0, "fileName": "" };
		$.each(uploaders, function (idx, up) {
			if (id == up.id) {
                var fileName = (typeof up.uploader.settings.multipart_params.upload_relevant_file_path == "undefined") ? 
                                       (up.uploader.files.length > 0 ? fileName : "") : 
                                       up.uploader.settings.multipart_params.upload_relevant_file_path;
				status = { "uploaded": up.uploader.total.uploaded, "failed": up.uploader.total.failed, "total": up.uploader.files.length, 
                           "fileName": fileName };
				return false;
			}
		});
		return status;
	}
    
	function startUpload() {
		// Collapse all configuration panels
		//$("#accordion .panel-collapse.in").collapse("hide");
		
		// Check valid inputs
		var missing = "";
		$.each(uploaders, function (idx, up) {
			var upStatus = uploadStatus(up.id);
			if (upStatus.total > upStatus.uploaded + upStatus.failed) {
				if (!validPanel(up.id)) {
					missing += (missing == "" ? "" : ", ") + ("`" + up.id + "`");
				}
			}
		});
		if (missing.length > 0) {
			alert("Please correct all invalid parameters for " + missing + " before uploading the selected files.");
			return false;
		}
		
		var pendingUploads = getNumberOfPendingUploads();
		if (pendingUploads == 0) {
			// Start LPIS/LUT processing
			var lpisStatus = uploadStatus("Lpis");
			var lutStatus  = uploadStatus("lut");
            var l4bCfgStatus  = uploadStatus("L4bCfg");
            var l4cCfgStatus  = uploadStatus("L4cCfg");
            var l4cCCStatus  = uploadStatus("L4cCCPractices");
            var l4cFLStatus  = uploadStatus("L4cFLPractices");
            var l4cNFCStatus  = uploadStatus("L4cNFCPractices");
            var l4cNAStatus  = uploadStatus("L4cCCPractices");
            var importFormStr = '<form action="create_site.php" method="post">';
            var formStrCreated = 0;
            
			if ((lpisStatus.uploaded + lutStatus.uploaded > 0) && (lpisStatus.failed + lutStatus.failed == 0)) {
                formStrCreated = 1;
                importFormStr = importFormStr +
									'<input type="hidden" name="lpis_start_import" value="LPIS Start Import">' +
									((lpisStatus.uploaded > 0 && lpisStatus.failed == 0) ?
									'<input type="hidden" name="lpis_on" value="1">' +
									'<input type="hidden" name="mode" value="' + $("#postUploadCmdParamLpisimportMode" ).val() + '">' +
									'<input type="hidden" name="parcelColumns" value="'  + encodeURIComponent($("#postUploadCmdParamLpisparcelIdCols" ).val()) + '">' +
									'<input type="hidden" name="holdingColumns" value="' + encodeURIComponent($("#postUploadCmdParamLpisholdingIdCols").val()) + '">' +
									'<input type="hidden" name="cropCodeColumn" value="' + encodeURIComponent($("#postUploadCmdParamLpiscropCodeCols" ).val()) + '">' +
									'<input type="hidden" name="lpisFile" value="' + encodeURIComponent(lpisStatus.fileName) + '">'
									: "") +
									((lutStatus.uploaded > 0 && lutStatus.failed == 0) ?
									'<input type="hidden" name="lut_on" value="1">' +
									'<input type="hidden" name="lutFile" value="' + encodeURIComponent(lutStatus.fileName) + '">'
									: "");
            }   

   			var l4bCfgStatus = uploadStatus("L4bCfg");
			if ((l4bCfgStatus.uploaded > 0) && (l4bCfgStatus.failed == 0)) {
                formStrCreated = 1;
				importFormStr = importFormStr +
									'<input type="hidden" name="l4bcfg_start_import" value="L4bCfg Start Import">' +
                                    '<input type="hidden" name="mowingStartDate" value="'  + encodeURIComponent($("#postUploadCmdParamL4bCfgmowingStartDate" ).val()) + '">' +
									'<input type="hidden" name="l4bCfgFile" value="' + encodeURIComponent(l4bCfgStatus.fileName) + '">';
            }

   			var l4cCfgStatus = uploadStatus("L4cCfg");
			if ((l4cCfgStatus.uploaded > 0) && (l4cCfgStatus.failed == 0)) {
                formStrCreated = 1;
				importFormStr = importFormStr +
									'<input type="hidden" name="l4ccfg_start_import" value="L4cCfg Start Import">' +
                                    '<input type="hidden" name="l4cCfgpractices" value="'  + encodeURIComponent($("#postUploadCmdParamL4cCfgpractices" ).val()) + '">' +
                                    '<input type="hidden" name="l4cCfgCountry" value="'  + encodeURIComponent($("#postUploadCmdParamL4cCfgcountry" ).val()) + '">' +
									'<input type="hidden" name="l4cCfgFile" value="' + encodeURIComponent(l4cCfgStatus.fileName) + '">';
            }
            var practicesArray = ["CC", "FL", "NFC", "NA"];
            for (var i = 0; i<practicesArray.length; i++) {
                var practiceName = practicesArray[i];
                var practiceStatus = uploadStatus("L4c" + practiceName + "Practices");
                if ((practiceStatus.uploaded > 0) && (practiceStatus.failed == 0)) {
                    formStrCreated = 1;
                    importFormStr = importFormStr +
                                        '<input type="hidden" name="' + practiceName.toLowerCase() + '_start_import" value="' + practiceName + ' Start Import">' +
                                        '<input type="hidden" name="l4c' + practiceName + 'PracticesFile" value="' + encodeURIComponent(practiceStatus.fileName) + '">';
                }
            }

            if (formStrCreated == 1) {
                importFormStr = importFormStr +
                                    '<input type="hidden" name="importSiteId" value="' + $("#edit_siteid").val() + '">' +
                                    '<input type="hidden" name="importYear" value="' + $("#upload_year").val() + '">';                
                importFormStr = importFormStr + '</form>';                
                $importForm = $(importFormStr);
				$.ajax({
					url: "create_site.php",
					type: "POST",
					data: $importForm.serialize(),
					success: function(response) {
						var needle = "all_import_done_response->";
						var n = response.indexOf(needle);
						if (n >= 0) {
							var jsonStr = response.substring(n + needle.length);
							try {
									var jsonObj = JSON.parse(jsonStr);
									if (jsonObj.status === "SUCCEEDED") {
										alert("SUCCESS: " + jsonObj.data);
										resetEditAdd("edit");
									} else {
										alert("FAILED: " + jsonObj.message);
									}
							} catch (e) {
								alert(e.message + "\n" + jsonStr);
							}
						} else {
							alert("Could not retrive a response from the server after LPIS/LUT import.");
						}
					},
					error: function (jqXHR, textStatus, errorThrown) {
						var msg = textStatus;
						if (jqXHR.responseJSON) {
							$.each(jqXHR.responseJSON, function (key, val) {
								msg += "\n" + key + ": " + val;
							});
						}
						alert(msg);
					}
				});
			}
		} else {
			// Proceed with uploading files/chunks
			doUploadChangedFiles(function() {
				// Recurent call to continue uploading remaining files/chunks
				startUpload();
			});
		}
	}
	
	$("#start_upload_files").on("click", function(e) {
		// Clear upload error messages
		$("#upload_msgs").val("");
		$("#err_up_files").val("");
		
		// Start upload
		startUpload();
	});
	
	// display OK/NOK message after the form has been posted
	<?php
	if ( isset($_SESSION['status']) ){
		if ( $_SESSION['status'] =='OK' ) {
			echo "alert('".$_SESSION['message']."')";
			unset($_SESSION['status']);
			unset($_SESSION['message']);
		} else if ( $_SESSION['status']=='NOK' && isset($_SESSION['result']) ){
			echo "alert('FAILED: ".$_SESSION['message']." ".$_SESSION['result'] ."')";
			unset($_SESSION['status']);
			unset($_SESSION['message']);
			unset($_SESSION['result']);
		}
	}
	
	if ($uploadYearChanged) { ?>
		$('tr[data-id="<?= $uploadSiteId ?>"] .link>a').click();
		$('input[name="upload_files"]').click();
	<?php
	}
	?>
});

// TODO : Use this version when removing completely the PHP
//function sendSatelliteEnableStatus() {
//    var l3bchkL8Btn = $("#chkL8Edit");
//    var siteId =  ($("#edit_siteid")[0].value);
//    var l8Enabled = $("#chkL8Edit")[0].checked;
//    var url = "http://localhost:8080/products/" + (l8Enabled ? "enable":"disable") + "/2/" + siteId;
//    $.ajax({
//        url: url,
//        type: "get",
//        cache: false,
//        crosDomain: true,
//        dataType: "html",
//        success: function(data) {
//
//        },
//        error: function (responseData, textStatus, errorThrown) {
//            alert("Satellite enable/disable was not performed! Error was " + errorThrown);
//        }
//    });
//}

function getSiteSeasons(site_id) {
	var collection = "table.table tr[data-id='" + site_id + "'] td.seasons";
	$(collection).each(function() {
		var season = this;
		$.ajax({
			url: "getSiteSeasons.php",
			type: "get",
			cache: false,
			crosDomain: true,
			data: { "siteId": $(season).parent().data("id"), "action": "get" },
			dataType: "html",
			success: function(data) {
				if (data.length > 0) {
					$(season).html(data);
					$(season).find("input[name='season_enabled']").bootstrapSwitch({
						size: "mini",
						onColor: "success",
						offColor: "default",
						disabled: true,
						handleWidth: 25
					});
				} else {
					$(season).html("-");
				}
			},
			error: function (responseData, textStatus, errorThrown) {
				console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			}
		});
	});
}

// TODO : Use this version when removing completely the PHP
//function getSiteSeasons(site_id) {
//    var collection = "table.table tr[data-id='" + site_id + "'] td.seasons";
//    $(collection).each(function() {
//        var season = this;
//        $.ajax({
//            url: "http://localhost:8080/sites/seasons/" + $(season).parent().data("id"),
//            type: "get",
//            cache: false,
//            crosDomain: true,
//            dataType: "html",
//            success: function(data) {
//                var tableElem = getSeasonsHtml(data);
//                if (tableElem != null) {
//                    $(season).html(tableElem);
//                    $(season).find("input[name='season_enabled']").bootstrapSwitch({
//                        size: "mini",
//                        onColor: "success",
//                        offColor: "default",
//                        disabled: true,
//                        handleWidth: 25
//                    });
//                } else {
//                    $(season).html("-");
//                }
//            },
//            error: function (responseData, textStatus, errorThrown) {
//                console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
//            }
//        });
//    });
//}


// TODO : Use this version when removing completely the PHP
//function getSeasonsHtml(json) {
//    var arr = $.parseJSON(json);
//    if (arr.length > 0) {
//        var tableElem = document.createElement('table');
//        tableElem.setAttribute("class", "subtable");
//        for (var i = 0; i<arr.length; i++) {
//            var trElem = document.createElement('tr');
//
//            var tdElem = document.createElement('td');
//            tdElem.innerHTML = arr[i].name;
//            trElem.appendChild(tdElem);
//
//            tdElem = document.createElement('td');
//            tdElem.innerHTML = formatDateFromJSonObj(arr[i].startDate);
//            trElem.appendChild(tdElem);
//
//            tdElem = document.createElement('td');
//            tdElem.innerHTML = formatDateFromJSonObj(arr[i].midDate);
//            trElem.appendChild(tdElem);
//
//            tdElem = document.createElement('td');
//            tdElem.innerHTML = formatDateFromJSonObj(arr[i].endDate);
//            trElem.appendChild(tdElem);
//
//            tdElem = document.createElement('td');
//            var input = document.createElement('input');
//            input.setAttribute("class", "form-control");
//            input.setAttribute("type", "checkbox");
//            input.setAttribute("name", "season_enabled");
//            input.setAttribute("checked", arr[i].enabled ? "checked" : "");
//            tdElem.appendChild(input);
//            trElem.appendChild(tdElem);
//
//            tableElem.appendChild(trElem);
//
//        }
//        return tableElem;
//    }
//    return null;
//}

function formatDateFromJSonObj(jsonObj) {
	return (jsonObj.year + "-" + ("0" + jsonObj.monthValue).slice(-2) + "-" + ("0" + jsonObj.dayOfMonth).slice(-2));
}

	
function validPanel(id) {
	var valid = true;
	$.each($("div.required>*:required ", "#panel" + id + ".uploading"), function () {
		if ($(this).val() == "") {
			$(this).parent().addClass("invalid");
			valid = false;
		} else {
			$(this).parent().removeClass("invalid");
		}
	});
	return valid;
}
function onNewFileSelected(id, titleId) {
	var str = $("#" + titleId).html();
	var modifIdx = str.indexOf('<font color');
	str = str.substring(0, modifIdx > 0 ? modifIdx : str.length);
	str = str + "<font color=\"red\"> (changed)</font>";
	$("#" + titleId).html(str);
	$("#" + titleId).closest(".panel").addClass("uploading");
	validPanel(id);
}
function onRemoveFile(id, titleId) {
	var str = $("#" + titleId).html();
	var modifIdx = str.indexOf('<font color');
	str = str.substring(0, modifIdx > 0 ? modifIdx : str.length);
	$("#" + titleId).html(str);
	$("#" + titleId).closest(".panel").removeClass("uploading");
	$.each(uploaders, function (idx, up) {
		if (id == up.id) {
			var cntFiles = up.uploader.files.length;
			for (i = cntFiles - 1; i >= 0; i --) {
				up.uploader.removeFile(up.uploader.files[i]);
			}
			return false;
		}
	});
	validPanel(id);
}
function onFileUploadEventStatus(titleId, msg) {
	var str = $("#" + titleId).html();
	var modifIdx = str.indexOf('<font color');
	str = str.substring(0, modifIdx > 0 ? modifIdx : str.length);
	str = str + "<font color=\"red\"> " + msg + "</font>";
	$("#" + titleId).html(str);
}

// Open add site form
function formAddSite(){
	// reset all form fields
	resetEditAdd("add");

	// open add site dialog and close all others
	$("#div_editsite").dialog("close");
	$("#div_deletesite").dialog("close");
	$("#div_uploadfiles").dialog("close");
	$("#div_addsite").dialog("open");
}

// open delete site dialog
function formDeleteSite(){
	// open delete site dialog and close all others
	$("#div_addsite").dialog("close");
	$("#div_uploadfiles").dialog("close");
	$("#div_deletesite").dialog("open");

	 // TODO : Use this version when removing completely the PHP
	//createDeleteCheckboxes();
}

// open upload files dialog
function formUploadFiles(){
	// open add site dialog and close all others
	$("#div_addsite").dialog("close");
	$("#div_deletesite").dialog("close");
	$("#div_uploadfiles").dialog("open");
}

 // TODO : Use this version when removing completely the PHP
//function createDeleteCheckboxes() {
//    $.ajax({
//        url: "http://localhost:8080/products/types/",
//        type: "get",
//        cache: false,
//        contentType: 'application/json',
//        mimeType: 'application/json',
//        crosDomain: true,
//        success: function(data) {
//            // sort the array by description
//            data.sort(function(a,b) {return (a.description > b.description) ? 1 : ((b.description > a.description) ? -1 : 0);} );
//            for (var i = 0; i<data.length; i++) {
//                var entry = data[i];
//                var shortName = data[i].description.substring(0,3);
//                var input = document.createElement('input');
//                input.setAttribute("class", "form-control");
//                input.setAttribute("id", "delete_chk" + shortName);
//                input.setAttribute("type", "checkbox");
//                input.setAttribute("name", "delete_chk" + shortName);
//                input.setAttribute("value", data[i].id);
//                input.setAttribute("checked", "checked");
//
//                var label = document.createElement('label');
//                label.setAttribute("class", "control-label");
//                label.setAttribute("for", "delete_chk" + shortName);
//                label.innerHTML = shortName;
//
//                var body = $("#delete_checkboxes_div")[0];
//                body.appendChild(input);
//                body.appendChild(label);
//            }
//        },
//        error: function (responseData, textStatus, errorThrown) {
//            alert ("Cannot extract product types! \r\n The Error was: " + errorThrown);
//        }
//    });
//}

// TODO : Use this version when removing completely the PHP
//function onDeleteSiteBtn() {
//    var selectedFiles = $('*[id^="delete_chk"]');
//    var siteShortName =  ($("#delete_site_short_name")[0].value);
//    var idsArr = [];
//    for (var i = 0; i<selectedFiles.length; i++) {
//        if (selectedFiles[i].checked) {
//            idsArr.push(parseInt(selectedFiles[i].value));
//        }
//    }
//    var postData = JSON.stringify({siteId:-1,siteShortName:siteShortName,productTypeIds:idsArr});
//    $.ajax({
//        url: "http://localhost:8080/sites/",
//        type: "delete",
//        data: postData,
//        cache: false,
//        contentType: 'application/json',
//        mimeType: 'application/json',
//        crosDomain: true,
//        success: function(data) {
//            alert ("Site successfully deleted!");
//            // close both the delete and edit site forms
//            $("#div_deletesite").dialog("close");
//            $("#div_editsite").dialog("close");
//            location.reload();
//        },
//        error: function (responseData, textStatus, errorThrown) {
//            //console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
//            alert ("Site couldn't be deleted! \r\n The Error was: " + errorThrown);
//            $("#div_deletesite").dialog("close");
//        }
//    });
//}

// Open edit site form
function formEditSite(id, name, short_name, site_enabled, existingUploadedFiles, l8Enabled) {
	// set values for all edited fields
<?php
	$uploadDescrs = getUploadFileDescriptorArray();
	foreach ($uploadDescrs as $uploadDescr) { ?>
		removeAppendedColoredText("site<?php echo $uploadDescr->id ?>DataAccHref");
	<?php } ?>

	$("#edit_sitename").val(name);
	$("#edit_siteid").val(id);
	$("#shortname").val(short_name);
<?php
	$upDescrs = array_values($uploadDescrs);
	foreach ($upDescrs as $index => $value) { ?>
		$("#site<?php echo $value->id ?>Data").val(existingUploadedFiles[<?php echo $index; ?>]);
	<?php } ?>
	$("#edit_enabled").bootstrapSwitch('state', site_enabled);
	$("#accordion").collapse('hide');
	$("#accordion").on("hidden.bs.collapse shown.bs.collapse", function (e) {
		$(this).closest(".ui-dialog").position({
			my: "center",
			at: "center",
			of: "#div_editsite",
			collision: "fit",
			// Ensure the titlebar is always visible
			using: function (pos) {
				var topOffset = $(this).css(pos).offset().top;
				if (topOffset < 0) {
					$(this).css("top", pos.top - topOffset);
				}
			}
		});
	});
	
	function repositionPanel($panel) {
	}
	
	$("#delete_sitename").val(name);
	$("#delete_site_short_name").val(short_name);

	$("#upload_sitename").val(name);
	$("#upload_siteid").val(id);
	
	// open edit site dialog and close all others
	$("#div_addsite").dialog("close");
	$("#div_editsite").data("id", id);
	$("#div_editsite").dialog("open");

	document.getElementById("chkL8Edit").checked = l8Enabled;
	// document.getElementById("chkL8Add").checked = l8Enabled;

	enableDisable( $('#chkS2Edit'),site_enabled);
	enableDisable( $('#chkL8Edit'),site_enabled);
  
	$.ajax({
		url: "getSiteSeasons.php",
		type: "get",
		cache: false,
		crosDomain: true,
		data:  { "siteId": id, "action": "edit" },
		dataType: "html",
		success: function(data) {
			$("#site-seasons").html(data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});


	//get tiles for L8
	$.ajax({
		type: "GET",
		url: "processing.php",
		dataType: "json",
		//data: {ajax: '1',site_id:id,satellite_id:2},
		data: {action: 'getTiles', siteId:id, satelliteId:2},
		success: function(data){
			$('textarea[name="L8Tiles"]').text(data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});

	//get tiles for S2
	$.ajax({
		type: "GET",
		url: "processing.php",
		dataType: "json",
		data: {action: 'getTiles', siteId:id, satelliteId:1},
		success: function(data){
			$('textarea[name="S2Tiles"]').text(data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
	
	// TODO : Use this version when removing completely the PHP
//    $.ajax({
//        url: "http://localhost:8080/products/enable/status/2/" + id,
//        type: "get",
//        cache: false,
//        crosDomain: true,
//        dataType: "html",
//        success: function(data) {
//            document.getElementById("chkL8Edit").checked = (data == "true") ? true : false;
//        },
//        error: function (responseData, textStatus, errorThrown) {
//            console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
//        }
//    });
};

function removeAppendedColoredText(id) {
	var str = document.getElementById(id).innerHTML;
	var modifIdx = str.indexOf('<font color');
	document.getElementById(id).innerHTML = str.substring(0, modifIdx > 0 ? modifIdx : str.length);
}

// Reset add/edit site event
function resetEditAdd(formName) {
	if (formName == "add") {
		var validator = $("#siteform").validate();
		validator.resetForm();
		$("#siteform")[0].reset();
	} else if (formName == "edit") {
		// reload sites page
		$(document.body).append('<form id="clone_do_nothing" action="create_site.php" method="post"></form>');
		$("form#clone_do_nothing").submit();
/*		
		//delete error messages for tiles
		$(".invalidTilesS2").text("");
		$(".invalidTilesL8").text("");
		
		var validator = $("#siteform_edit").validate();
		validator.resetForm();
		$("#siteform_edit")[0].reset();
		
		// refresh seasons for edited/added site
		var site_id = $("#div_editsite").data("id");
		getSiteSeasons(site_id);
		$("#div_editsite").removeData("id");
		
		$( ".create-site tr").removeClass("editing");
*/
	} else if (formName == "delete") {
		//$("#siteform_delete")[0].reset();
	} else if (formName == "upload") {
		//$("#siteform_upload")[0].reset();
	}
}

// Abort add/edit site event
function abortEditAdd(abort) {
	if (abort == 'add') {
		$("#div_addsite").dialog("close");
	} else if (abort == 'edit') {
		$("#div_editsite").dialog("close");
	} else if (abort == 'delete_site') {
		$("#div_deletesite").dialog("close");
	} else if (abort == 'upload_files') {
		$("#div_uploadfiles").dialog("close");
	}
}

function enableDisable(elem,site_enabled) {
	var value = elem.val();
	if (site_enabled) {
		if (elem.is(':checked')) {
			//enable textarea for editing tiles
			$('textarea[name="'+value+'Tiles"]').prop('disabled', false);
		} else {
			//disable textarea for editing tiles
			$('textarea[name="'+value+'Tiles"]').prop('disabled', true);
		}
	} else {
		$('textarea[name="'+value+'Tiles"]').prop('disabled', true);
	}
}

function createUploaders() {
	var uploaders = new Array();
	<?php
	$uploadDescrs = getUploadFileDescriptorArray();
	foreach ($uploadDescrs as $uploadDescr) {
		?>
		var uploader = createUploader("<?php echo $uploadDescr->id ?>", "<?php echo $uploadDescr->expectedUploadFileExt ?>");
		<?php
		if ($uploadDescr->addParams) {
			$jsonParams = json_decode($uploadDescr->addParams);
			foreach ($jsonParams as $param) {
				$paramId = $uploadDescr->id . $param->id;
				?>
				uploader.addPostUploadCmdParam("postUploadCmdParam<?php echo $paramId?>");
				<?php
			}
		} ?>
		uploaders.push(uploader);
		<?php
	} ?>
	return uploaders;
}

class UploaderWrapper {
	constructor(id, uploader) {
		this._id = id;
		this._uploader = uploader;
		this._postUploadCmdParams = new Array();
	}
	get id() {
		return this._id;
	}
	get uploader() {
		return this._uploader;
	}
	
	addPostUploadCmdParam(paramId) {
		this._postUploadCmdParams.push(paramId);
	}
	
	get postUploadCmdParams() {
		return this._postUploadCmdParams;
	}
}

function checkAndHandleErrorMsg(up, file, info, isChunk) {
	var errIdx = info.response.indexOf("{\"upload_response\" : {\"error\" : {\"code\":");
	if (errIdx >= 0) {
		var errMsg = info.response.substring(errIdx);
		var jsonObj = JSON.parse(errMsg);
		file.status = plupload.FAILED;
		//up.trigger('UploadProgress', file);
		up.trigger('QueueChanged');
		up.trigger('Error', {
			code: jsonObj.upload_response.error.code,
			message: ((isChunk ? 'Upload error for chunk of file ' : 'Upload error for file ') + file.name + " : "  + jsonObj.upload_response.error.message),
			file: file,
			status: 0
		});
		// In this case, the state changed is not triggered so we need to trigger it manually
		if (isChunk) {
			up.trigger('StateChanged');
		}
		return true;
	}
	return false;
}

function addKeyValToJsonArr(curJson, key, value) {
	if (curJson == "") {
		curJson = "[]";
	}
	var jsonObj = JSON.parse(curJson);
	jsonObj.push("{\"key\":\"" + key + "\", \"value\":\"" + value + "\"}");
	return JSON.stringify(jsonObj);
}

function createUploader(id, fileExt) {
	if (fileExt.startsWith(".")) {
		fileExt = fileExt.slice(1);
	}
	var uploader = new plupload.Uploader({
		runtimes : 'html5,flash,silverlight,html4',
		max_files: 1,
		browse_button : 'pickfiles'  + id,
		container: document.getElementById('container' + id),
		url : "/create_site.php",
		chunk_size : '7mb',
		filters : {
			mime_types: [
				//{title : "Image files", extensions : "jpg,gif,png"},
				{title : "Files", extensions : fileExt}
			]
		},
		// Flash settings
		flash_swf_url : '/plupload/js/Moxie.swf',
		// Silverlight settings
		silverlight_xap_url : '/plupload/js/Moxie.xap',
		multipart_params : {
			"upload_file" : "1",
			"upload_year" : "<?= $uploadYear ?>"
		},
		init: {
			PostInit: function () {
				$("#filelist" + id).val('');
			},
			FilesAdded: function (up, files) {
				if (up.files.length >= up.settings.max_files) {
					up.splice(up.settings.max_files);
					$(up.settings.browse_button).hide();
				}
				plupload.each(up.files, function (file) {
					$("#filelist" + id + ".upload-file").css({ "padding-left": "10px", "width": "300px" });
					$(up.settings.container).find("input[type=file]").attr("title", "");
					$("#filelist" + id).val(file.name + ' (' + plupload.formatSize(file.size) + ')').change();
				});
			},
			FilesRemoved: function (up, files) {
				if (up.files.length < up.settings.max_files) {
					$("#filelist" + id).val('');
					$("#filelist" + id + ".upload-file").css({ "padding-left": "", "width": "" });
					$(up.settings.browse_button).show();
				}
			},
			UploadProgress: function (up, file) {
				onFileUploadEventStatus("site" + id + "DataAccHref", file.percent + " %");
			},
			FileUploaded: function (up, file, info) {
				// Called when file has finished uploading and if the upload was a success by checking also the result
				if (!checkAndHandleErrorMsg(up, file, info, false)) {
					onFileUploadEventStatus("site" + id + "DataAccHref", "Upload finished successfully!");
					var n = info.response.indexOf("{\"upload_response\" : {");
					var jsonStr = info.response.substring(n);
					var jsonObj = JSON.parse(jsonStr);
                    up.settings.multipart_params.upload_relevant_file_path   = jsonObj.upload_response.upload_relevant_file_path;                    
					$("#succ_up_files").val(addKeyValToJsonArr($("#succ_up_files").val(), id, file.name));
				}
			},
			ChunkUploaded: function (up, file, info) {
				if (!checkAndHandleErrorMsg(up, file, info, true)) {
					// Called when file chunk has finished uploading
					var n = info.response.indexOf("{\"upload_response\" : {");
					var jsonStr = info.response.substring(n);
					var jsonObj = JSON.parse(jsonStr);
					up.settings.multipart_params.upload_target_dir = jsonObj.upload_response.upload_target_dir;
					up.settings.multipart_params.upload_rel_path   = jsonObj.upload_response.upload_rel_path;
				}
			},
			Error: function(up, err) {
				onFileUploadEventStatus("site" + id + "DataAccHref", "Error #" + err.code + ": " + err.message);
				var curMsg = $("#upload_msgs").val();
				var str = curMsg + (curMsg == "" ? "" : "\n") + err.message;
				$("#upload_msgs").val(str);
				$("#err_up_files").val(addKeyValToJsonArr($("#err_up_files").val(), id, err.message));
			}
		}
	});
	
	uploader.init();
	return new UploaderWrapper(id, uploader);
}

function doUploadFile(id) {
	var uploader = null;
	var upWrp = getUploaderWrp(id);
	if (upWrp != null) {
		var uploader = upWrp.uploader;
		uploader.settings.multipart_params.site_shortname = $("#shortname").val();
		uploader.settings.multipart_params.upload_desc_id = id;
		// fill the upload parameters if supported in the upload descriptor
		var cmdsParams = upWrp.postUploadCmdParams;
		for (var i = 0; i<cmdsParams.length; i++) {
			var e = document.getElementById(cmdsParams[i]);
			var obj = {};
			var key = cmdsParams[i];
			var selOpt = null;
			if (e.tagName.toUpperCase() === "SELECT") {
				selOpt = e.options[e.selectedIndex].value;
			} else {
				// Text only supported now
				selOpt = e.value;
			}
			obj[key] = selOpt;
			$.extend(uploader.settings.multipart_params, obj);
		}
		uploader.start();
	}
	
	return uploader;
}

function isUploaderDoneForAllUploads(uploader) {
	if( (uploader.files.length > 0) &&
		(uploader.files.length != (uploader.total.uploaded + uploader.total.failed))) {
		return false;
	}
	return true;
}

function getNumberOfPendingUploads() {
	var cntSendingUploaders = 0;
	var len = uploaders.length;
	for(var idx = 0; idx < len; idx++) {
		var id = uploaders[idx].id;
		var uploader = uploaders[idx].uploader;
		var str = $("#filelist" + id).val();
		if ((str != "") && !isUploaderDoneForAllUploads(uploader)) {
			cntSendingUploaders++;
		}
	}
	return cntSendingUploaders;
}

function doUploadChangedFiles(callback) {
	// get the modified files
	var cntSendingUploaders = 0;
	
	var len = uploaders.length;
	for(var idx = 0; idx < len; idx++) {
		var id = uploaders[idx].id;
		var uploader = uploaders[idx].uploader;
		var str = $("#filelist" + id).val();
		if ((str != "") && !isUploaderDoneForAllUploads(uploader)) {
			cntSendingUploaders++;
			uploader = doUploadFile(id);
			uploader.unbind('StateChanged');
			uploader.bind('StateChanged', function() {
				var allFinished = true;
				for (var i = 0; i < len; i++) {
					if (!isUploaderDoneForAllUploads(uploaders[i].uploader)) {
						allFinished = false;
					}
				}
				if (allFinished == true) {
					callback();
				}
			});
		}
	}
	// if no uploading file selected, then call the callback anyway
	if (cntSendingUploaders == 0) {
		callback();
	}
		
	return true;
}

function getUploaderWrp(id) {
	var len = uploaders.length;
	for (var i = 0; i < len; i++) {
		if (uploaders[i].id == id) {
			return uploaders[i];
		}
	};
	return null;
}

function getUploader(id) {
	var upWrp = getUploaderWrp(id);
	if (upWrp != null) {
		return upWrp.uploader;
	}
	return null;
}

var uploaders = createUploaders();
//# sourceURL=create_site.js
</script>

<?php include 'ms_foot.php'; ?>
