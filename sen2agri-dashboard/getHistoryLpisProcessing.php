<?php
	require_once("ConfigParams.php");
	
	$curl = curl_init();
	$url =  ConfigParams::$REST_SERVICES_URL . "/progress/";
	
	curl_setopt($curl, CURLOPT_URL,  $url );
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
	
	$result = curl_exec($curl);
	$result = json_decode($result);
	$tr_current = "";
	if(!empty($result) && (empty($result->status) || $result->status != "SUCCEEDED")) {
		$tr_current = "<tr><td colspan='3'>Failed to retrieve processing status.</td></tr>";
	} else if(!empty($result) && !empty($result->data)) {
		foreach ($result->data as $res) {
			$subtr = "";
			if ($res->subTaskProgress != null) {
				$subcnt = 1;
				foreach ($res->subTaskProgress as $subres) {
					$progress = ($subres->progress!= 0)? $subres->progress*100 : 0;
					$class =  ($subres->progress!= 0)? "progress-bar-success" : "progress-bar-zero";
					$tr = "<tr>".
							"<td>" . $subres->name . "</td>".
							"<td class='td_progress'>
								<div class=\"progress\" style=\"color:red\">
								  <div class=\"progress-bar ".$class."\" role=\"progressbar\"  
								  aria-valuemin=\"0\" aria-valuemax=\"100\" style=\"width:".$progress ."%\">".$progress." %
								  </div>
								</div>
						   </td>".
						"</tr>";
					$subtr .= $tr;
					$subcnt ++;
				}
				$tr = "<tr><td rowspan=" . $subcnt . ">" . $res->name . "</td></tr>" . $subtr;
			} else {
				$progress = ($res->progress!= 0)? $res->progress*100 : 0;
				$class =  ($res->progress!= 0)? "progress-bar-success" : "progress-bar-zero";
				$tr = "<tr>".
						"<td>" . $res->name . "</td>".
						"<td>-</td>".
						"<td class='td_progress'>
							<div class=\"progress\" style=\"color:red\">
							  <div class=\"progress-bar ".$class."\" role=\"progressbar\"  
							  aria-valuemin=\"0\" aria-valuemax=\"100\" style=\"width:".$progress ."%\">".$progress." %
							  </div>
							</div>
					   </td>".
					"</tr>";
			}
			$tr_current = $tr_current . $tr;
		}
	} else {
		$tr_current = "<tr><td colspan='3'>No processing operations in progress.</td></tr>";
	}
	
	curl_close($curl);
	echo $tr_current;
	
?>