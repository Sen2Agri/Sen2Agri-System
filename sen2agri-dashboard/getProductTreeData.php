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
	$dbconn       = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	
	$site_id = ConfigParams::$SITE_ID;
	if(isset($_POST['site_id']) && $_POST['site_id']!=""){
		$site_id = $_POST['site_id'];
	}
	if(strlen($site_id)==0) $site_id = null;
	
	$satellit_id = null;
	
	$season_id = null;
	if(isset($_POST['season_id']) && $_POST['season_id']!=""){
		$season_id = $_POST['season_id'];
	}
	
	$product_type_id = null;
	if(isset($_POST['product_id'])){
		$product_type_id = '{'.implode(', ',$_POST['product_id']).'}';
	}
	
	$from = null;
	if(isset($_POST['start_data']) && $_POST['start_data']!=""){
		$from = $_POST['start_data'];
	}
	
	$to = null;
	if(isset($_POST['end_data']) && $_POST['end_data']!=""){
		$to = $_POST['end_data'];
	}
	
	$tiles = null;
	if(isset($_POST['tiles']) && $_POST['tiles']!=""){
		$tiles = '{'.implode(', ',$_POST['tiles']).'}';
		
	}
	$rows         = pg_query_params($dbconn, "select * from sp_get_dashboard_products($1,$2,$3,$4,$5,$6,$7)",array($site_id,$product_type_id,$season_id,$satellit_id,$from,$to,$tiles)) or die(pg_last_error());
	$responseJson = pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "";
	$productRows  = json_decode($responseJson);

	if(isset($_POST['satellit_id'])){
	   $satellit_id = $_POST['satellit_id'];
	 }
	
	if($productRows!=null){
	    foreach($productRows as $productRow) {
	        
	        //if satellite_id is set then apply it on product type "l2a"
	        if(isset($_POST['satellit_id']) && $productRow->product_type_id == '1'){
	            if(!in_array($productRow->satellite_id, $satellit_id)){
	                continue;
	            }
	        }
	        
	        $siteObj = findSite($productRow->site, $products);
			if ($siteObj == null) {
				$siteObj = new stdClass();
				$siteObj->text = $productRow->site;
				$siteObj->selectable = false;
				$siteObj->nodes = array();
				$products[] = $siteObj;
			}
	
			$prodTypeObj = findProdType($productRow->product_type_description, $siteObj->nodes);
			if ($prodTypeObj == null) {
				$prodTypeObj = new stdClass();
				$prodTypeObj->text = $productRow->product_type_description;
				$prodTypeObj->selectable = false;
				$prodTypeObj->nodes = array();
				$siteObj->nodes[] = $prodTypeObj;
			}
	
			if ($productRow->product != null) {
				$productObj = new stdClass();
				$productObj->productId = $productRow->product;
				$productObj->text = $productRow->product;
				$productObj->href = "downloadProduct.php?id=".$productRow->id;
				$productObj->productCoord = array_map('floatval', explode(",", str_replace(array("(", ")"), "", $productRow->footprint)));
				$productObj->siteCoord = $productRow->site_coord;
	
				$productObj->productImageUrl = "getProductImage.php?id=".$productRow->id;
	
				$prodTypeObj->nodes[] = $productObj;
			}
	
	    }
	}

} catch (Exception $e) {
    $products = null;
}
echo json_encode($products, JSON_UNESCAPED_SLASHES);

?>
