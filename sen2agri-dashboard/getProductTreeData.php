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
	$rows         = pg_query($dbconn, "select * from sp_get_dashboard_products(".ConfigParams::$SITE_ID.")") or die(pg_last_error());
	$responseJson = pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "";
	$productRows  = json_decode($responseJson);

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

			$prodTypeObj->nodes[] = $productObj;
		}

    }

} catch (Exception $e) {
    $products = null;
}
echo json_encode($products, JSON_UNESCAPED_SLASHES);

?>
