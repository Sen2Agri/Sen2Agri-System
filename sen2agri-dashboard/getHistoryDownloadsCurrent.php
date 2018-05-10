<?php
	require_once("ConfigParams.php");
	
	$curl = curl_init();
	$url =  ConfigParams::$REST_SERVICES_URL . "/downloader/";
	if (isset($_REQUEST['siteID_selected'])) {
		$url.= $_REQUEST['siteID_selected'] ;
	}
	
	curl_setopt($curl, CURLOPT_URL,  $url );
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
	
	$result = curl_exec($curl);
	$result = json_decode($result);
	$tr_current ="";
	if(!empty($result)){
		foreach ($result as $res){
			$tr = "<tr>".
				"<td>" . $res->siteName . "</td>".
				"<td>" . $res->name. "</td>".
				"<td>" . $res->satelliteName. "</td>".
				"<td>" . $res->progress*100 . "%</td>".
				"</tr>";
			$tr_current = $tr_current . $tr;
		}
	}else{
		$tr_current = "<tr><td colspan='4'>No downloads in progress.</td></tr>";
	}
	
	curl_close($curl);
	echo $tr_current;
	
?>