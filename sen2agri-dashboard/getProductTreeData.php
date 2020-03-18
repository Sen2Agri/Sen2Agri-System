<?php
session_start();
require_once ("ConfigParams.php");

function findSite($siteName, $siteObjArr) {
	foreach($siteObjArr as $siteObj) {
		if ($siteObj->text == $siteName)
			return $siteObj;
	}

	return null;
}

function findProcessor($processorName, $processorObjArr) {
	foreach($processorObjArr as $processorObj) {
		if ($processorObj->text == $processorName)
			return $processorObj;
	}

	return null;
}

function findProdType($prodTypeName, $prodTypeObjArr) {
	foreach($prodTypeObjArr as $prodTypeObj) {
		if ($prodTypeObj->text == $prodTypeName)
			return $prodTypeObj;
	}

	return null;
}

function normalizePath($path) {
    $path = str_replace('\\', '/', $path);
    $path = preg_replace('/\/+/', '/', $path);
    return $path;
}

$products = array();

try {
    $dbconn       = pg_connect(ConfigParams::getConnection()) or die ("Could not connect");
	
	if($_SESSION['isAdmin'] || sizeof($_SESSION['siteId'])>0){
	    
    	$site_id = empty(ConfigParams::$SITE_ID) ? "":"{".implode(',',ConfigParams::$SITE_ID)."}";
    	if(isset($_REQUEST['site_id']) && $_REQUEST['site_id']!=""){
    	    $site_id = "{".$_REQUEST['site_id']."}";
    	}
    	if(strlen($site_id)==0) $site_id = null;
    	
    	$satellit_id = null;
        
        $userName = $_SESSION['userName'];
    	
    	$season_id = null;
    	if(isset($_REQUEST['season_id']) && $_REQUEST['season_id']!=""){
    	    $season_id = $_REQUEST['season_id'];
    	}
    	
    	$product_type_id = null;
    	if(isset($_REQUEST['product_id'])){
    	    $product_type_id = '{'.implode(', ',$_REQUEST['product_id']).'}';
    	}
    	
    	$from = null;
    	if(isset($_REQUEST['start_data']) && $_REQUEST['start_data']!=""){
    	    $from = $_REQUEST['start_data'];
    	}
    	
    	$to = null;
    	if(isset($_REQUEST['end_data']) && $_REQUEST['end_data']!=""){
    	    $to = $_REQUEST['end_data'];
    	}
    	
    	$tiles = null;
    	if(isset($_REQUEST['tiles']) && $_REQUEST['tiles']!=""){
    	    $tiles = '{'.implode(', ',$_REQUEST['tiles']).'}';
    		
    	}
    	
    	if(isset($_REQUEST['action']) && $_REQUEST['action']=='getNodes'){ //get level 3 for treeview
    	
    	    $rows         = pg_query_params($dbconn, "select * from sp_get_dashboard_products_nodes($1,$2,$3,$4,$5,$6,$7,$8,$9)",array($userName,$site_id,$product_type_id,$season_id,$satellit_id,$from,$to,$tiles,true)) or die(pg_last_error());
    	  
    	    $responseJson = pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "";
    	    $productRows  = json_decode($responseJson); 
    	    
    	    $products = new stdClass();
    	    if($productRows!=null){
    	        foreach($productRows as $productRow) {
    	            if ($productRow->product != null) {
    	                if(isset($_REQUEST['satellite_id']) ){
    	                    $satellite_id = $_REQUEST['satellite_id'];
    	                }
    	          
    	                if(isset($_REQUEST['satellite_id']) && $productRow->product_type_id == '1'){
    	                    if(!in_array($productRow->satellite_id, $satellite_id)){ 
    	                        continue;
    	                    }
    	                }
    	                
        	            $productObj = new stdClass();
        	            $productObj->text = $productRow->product;
        	            $productObj->href = "downloadProduct.php?id=".$productRow->id;
        	            $productObj->productCoord = array_map('floatval', explode(",", str_replace(array("(", ")"), "", $productRow->footprint)));
        	            $productObj->siteCoord = $productRow->site_coord;
        	            
        	            $productObj->productId =$productRow->id;
        	            
        	            if(!ConfigParams::isSen2Agri())
        	            	$productObj->is_raster = $productRow->is_raster;
        	            
        	            $products->nodes[] = $productObj;
    	            }

    	        }
    	    }
    	    
    	}else{//get the first 2 levels for treeview
        	$rows         = pg_query_params($dbconn, "select * from sp_get_dashboard_products_nodes($1,$2,$3,$4,$5,$6,$7,$8)",array($userName,$site_id,$product_type_id,$season_id,$satellit_id,$from,$to,$tiles)) or die(pg_last_error());
        	
        	$responseJson = pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "";
        	$productRows  = json_decode($responseJson); 
        	
        	if(isset($_REQUEST['satellite_id'])){
        	    $satellite_id = $_REQUEST['satellite_id'];
        	}
        	
        	if($productRows!=null){
        	    foreach($productRows as $productRow) {
        	        
        	        //if satellite_id is set then apply it on product type "l2a"
        	        if(isset($_REQUEST['satellite_id']) && $productRow->product_type_id == '1'){
        	            if(!in_array($productRow->satellite_id, $satellite_id)){
        	                continue;
        	            }
        	        }
        	        
        	        $siteObj = findSite($productRow->site, $products);
        			if ($siteObj == null) {
        				$siteObj = new stdClass();
        				$siteObj->text = $productRow->site;
        				$siteObj->selectable = false;
        				$siteObj->nodes = array();
        				$siteObj->id = $productRow->site_id;
        				$products[] = $siteObj;
        			}
        	
        			$prodTypeObj = findProdType($productRow->product_type_description, $siteObj->nodes);
        			if ($prodTypeObj == null) {
        				$prodTypeObj = new stdClass();
        				$prodTypeObj->text = $productRow->product_type_description;
        				$prodTypeObj->selectable = false;
        				$prodTypeObj->nodes = array();
        				$prodTypeObj->id = $productRow->product_type_id;
        				$siteObj->nodes[] = $prodTypeObj;
        			}
        	
        	    }
        	}
    	}
	}

} catch (Exception $e) {
    $products = null;
}
echo json_encode($products, JSON_UNESCAPED_SLASHES);

?>
