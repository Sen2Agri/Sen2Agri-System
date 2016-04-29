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

function getProductImageSize($file) {
	if (file_exists($file)) {
		$image_info = getImageSize($file);
		return array('width'=>$image_info[0], 'height'=>$image_info[1]);
	} else {
		return array('width'=>null, 'height'=>null);
	}
}

$products = array();

try {
	$URL = ConfigParams::$SERVICES_DASHBOARD_PRODUCTS_URL;
	$PRODUCT_ROOT_FOLDER = ConfigParams::$PRODUCT_ROOT_FOLDER;
	$SITE_PRODUCT_RELATIVE_FOLDER = ConfigParams::$SITE_PRODUCT_RELATIVE_FOLDER;
	
	//Needs extra lib
	//$responseJson = http_get($URL);
	
	$ch = curl_init();
	$timeout = 5;
	
	curl_setopt($ch, CURLOPT_URL, $URL);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, $timeout);
	
	$responseJson = curl_exec($ch);
	curl_close($ch);
	
	$productRows = json_decode($responseJson);
	
    foreach($productRows as $productRow) {
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
			$imageSize = getProductImageSize(rtrim(normalizePath($productRow->full_path), '/').'/'.$productRow->quicklook_image);
			$productObj->productImageWidth = $imageSize['width'];
			$productObj->productImageHeight = $imageSize['height'];
			
			$prodTypeObj->nodes[] = $productObj;
		}
		
    }
	
} catch (Exception $e) {
    $products = null;
}
echo json_encode($products, JSON_UNESCAPED_SLASHES);

?>
