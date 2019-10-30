<?php
	require_once("ConfigParams.php");
	
	$curl = curl_init();
	$url =  ConfigParams::$REST_SERVICES_URL . "/progress/?category=lpis.progress";
	if (isset($_REQUEST['siteID_selected']) && $_REQUEST['siteID_selected'] != '0') {
		$url .= '&filter={"name":"siteId","value":"' . $_REQUEST['siteID_selected'] . '"}';
	}
	
	curl_setopt($curl, CURLOPT_URL,  $url );
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
	
	$resultJson = curl_exec($curl);
	//$resultJson = '{"data":[{"name":"LTU_2019_07_02_Decl.shp Sen4CAP_L4A_LTU_2019_CropCode_LUT.csv","category":"lpis.progress","progress":0.0,"subTaskProgress":{"key":"Finding duplicate parcels 3","value":70.0},"info":{"siteId":"12","siteName":"Test","Payload":"Started LTU_2019_07_02_Decl.shp Sen4CAP_L4A_LTU_2019_CropCode_LUT.csv:Finding duplicate parcels 3","Topic":"lpis.progress","Principal":"SystemAccount"}},{"name":"LTU_2016_07_02_Decl.shp Sen4CAP_L4A_LTU_2018_CropCode_LUT.csv","category":"lpis.progress","progress":0.5465165,"subTaskProgress":null,"info":{"siteId":"12","siteName":"Test","Payload":"Started LTU_2015_07_02_Decl.shp Sen4CAP_L4A_LTU_2018_CropCode_LUT.csv","Topic":"lpis.progress","Principal":"SystemAccount"}},{"name":"LTU_2018_07_02_Decl.shp Sen4CAP_L4A_LTU_2018_CropCode_LUT.csv","category":"lpis.progress","progress":0.0,"subTaskProgress":{"key":"Finding duplicate parcels x","value":30.0},"info":{"siteId":"12","siteName":"Test","Payload":"Started LTU_2018_07_02_Decl.shp Sen4CAP_L4A_LTU_2018_CropCode_LUT.csv:Finding duplicate parcels x","Topic":"lpis.progress","Principal":"SystemAccount"}},{"name":"LTU_2019_07_02_Decl.shp Sen4CAP_L4A_LTU_2019_CropCode_LUT.csv","category":"lpis.progress","progress":0.0,"subTaskProgress":{"key":"Finding duplicate parcels 1","value":30.0},"info":{"siteId":"12","siteName":"Test","Payload":"Started LTU_2019_07_02_Decl.shp Sen4CAP_L4A_LTU_2019_CropCode_LUT.csv:Finding duplicate parcels 1","Topic":"lpis.progress","Principal":"SystemAccount"}}],"message":null,"status":"SUCCEEDED"}';
	$result = json_decode($resultJson);
	
	//echo "<tr><td><a target='blank' href='".$url."'>".$url."</a></td><td colspan=3>".$resultJson."</td></tr>";
	
	$tr_current = "";
	if(!empty($result) && (empty($result->status) || $result->status != "SUCCEEDED")) {
		$tr_current = "<tr><td colspan='4'>Failed to retrieve processing operations.</td></tr>";
	} else if(!empty($result) && !empty($result->data)) {
		// Sort results by "name + subTaskProgress->name"
		usort($result->data, function($a, $b) {
			if (!empty($a->subTaskProgress) && !empty($a->subTaskProgress) && ($a->name == $b->name)) {
				return $a->subTaskProgress->key > $b->subTaskProgress->key ? 1 : -1;
			} else {
				return $a->name > $b->name ? 1 : -1;
			}
		});
		$parent = "---";
		$cnt = 0;
		foreach ($result->data as $res) {
			if (!empty($res->category) && $res->category == "lpis.progress") {
				if ($res->name != $parent) {
					$tr_current = str_replace("<tr><td class='first'>",  "<tr><td rowspan=". $cnt .">", $tr_current);
					$parent = $res->name;
					$cnt = 0;
				}
				$name     = $res->name;
				$chname   = "-";
				$info     = $res->info;
				$progress = ($res->progress != 0) ? $res->progress*100 : 0;
				$class    = ($res->progress != 0) ? "progress-bar-success" : "progress-bar-zero";
				if ($res->subTaskProgress != null) {
					$chname   = $res->subTaskProgress->key;
					$progress = ($res->subTaskProgress->value != 0)? $res->subTaskProgress->value : 0;
					$class    = ($res->subTaskProgress->value != 0)? "progress-bar-success" : "progress-bar-zero";
				}
				$tr = "<tr>".
						($cnt == 0 ? ("<td class='first'>" . $name . "</td>") : "") .
						"<td>" . $chname . "</td>".
						"<td class='td_progress'>
							<div class=\"progress\" style=\"color:red\">
							  <div class=\"progress-bar ".$class."\" role=\"progressbar\"  
							  aria-valuemin=\"0\" aria-valuemax=\"100\" style=\"width:".$progress ."%\">".$progress." %
							  </div>
							</div>
					   </td>".
					"</tr>";
				$tr_current = $tr_current . $tr;
				$cnt ++;
			}
		}
		if ($cnt > 0) {
			$tr_current = str_replace("<tr><td class='first'>",  "<tr><td rowspan=". $cnt .">", $tr_current);	
		}
	} else {
		$tr_current = "<tr><td colspan='4'>No processing operations in progress.</td></tr>";
	}
	
	curl_close($curl);
	echo $tr_current;
?>