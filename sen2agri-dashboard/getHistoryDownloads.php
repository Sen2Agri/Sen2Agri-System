<?php
	require_once("ConfigParams.php");
	
	function getHistoryDownloads($site_id){
    	if (isset($_REQUEST['siteID_selected'])) {
    	    $db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
    		$result = "";
    		if ($_REQUEST['siteID_selected'] == 0) {
    			$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history(null)", array () ) or die ( "Could not execute." );
    		} else {
    			$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history($1)", array ($_REQUEST['siteID_selected']) ) or die ( "Could not execute." );
    		}
    	
    		$numbers = array(0,0,0,0);
    		$percentage = array(0,0,0,0);
    		while ( $row = pg_fetch_row ( $result ) ) {
    			if ($row[0] == 1) {
    				$numbers[0] = $row[1];
    				$percentage[0] = $row[2];
    			} else if($row[0] == 2){
    			    $numbers[1] = $row[1];
    			    $percentage[1] = $row[2];
    			} else if($row[0] == 3){
    			    $numbers[2] = $row[1];
    			    $percentage[2] = $row[2];
    			}
    			else if($row[0] == 4){
    			    $numbers[3] = $row[1];
    			    $percentage[3] = $row[2];
    			}
    		}
    		 echo json_encode(array('numbers'=>$numbers,'percentage'=>$percentage));
    	}
	}
	
	function getLpisGsaaDownloads($site_id){
	    $curl = curl_init();
	    $url =  ConfigParams::$REST_SERVICES_URL . "/progress/";
	    curl_setopt($curl, CURLOPT_URL,  $url );
	    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
	    
	    $result = curl_exec($curl);
	    $result = json_decode($result);    
	    $httpcode = curl_getinfo($curl,CURLINFO_HTTP_CODE);
	    
	    // Check if any error occurred
	    if (!curl_errno($curl) && ($httpcode>=200 && $httpcode<300)) {
	      
	        if(!empty($result)){
	            foreach ($result as $res){
	                $progress = ($res->progress!= 0)? $res->progress*100 : 0;
	                $class =  ($res->progress!= 0)? "progress-bar-success" : "progress-bar-zero";
	                $tr = "<tr>".
	   	                "<td>" . $res->siteName . "</td>".
	   	                "<td>" . $res->file. "</td>".
	   	                "<td>" . $res->fileType. "</td>".
	   	                "<td class='td_progress'>
                    <div class=\"progress\" style=\"color:red\">
                      <div class=\"progress-bar ".$class."\" role=\"progressbar\"
                      aria-valuemin=\"0\" aria-valuemax=\"100\" style=\"width:".$progress ."%\">".$progress." %
                      </div>
                    </div>
               </td>".
               "</tr>";
	                $tr_current = $tr_current . $tr;
	            }
	        }else{
	            echo  "<tr><td colspan='4'>No downloads in progress.</td></tr>";
	        }

	    }else{
	        echo "<tr><td colspan='4'>Something went wrong.Please check that the sen2agri-services are started!</td></tr>";
	    }
	    
	    curl_close($curl);
	}
	
	
	if ($_SERVER['REQUEST_METHOD'] === 'GET') {
	    // GET actions
	    $result = "";
	    $action = $_GET["action"];
	    switch ($action) {
	        case 'historyDownloads': getHistoryDownloads($_REQUEST['siteID_selected']);
	           break;
	        case 'lpisGsaaDownloads': getLpisGsaaDownloads($_REQUEST['site_id']);
	           break;
	    }
	}

?>
