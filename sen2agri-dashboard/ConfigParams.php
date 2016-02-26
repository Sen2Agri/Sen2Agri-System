<?php

class ConfigParams {

	static $SERVICES_BASE_URL = 'http://sen2agri-dev:8080/dashboard';
	static $WEB_BASE_URL = 'http://sen2agri-dev';
	
	static $PRODUCT_ROOT_FOLDER = '/mnt/output';
	static $SITE_PRODUCT_RELATIVE_FOLDER = 'files/Sen2AgriFiles';
	
	static $SERVICES_DASHBOARD_PRODUCTS_URL;
	static $SERVICES_DASHBOARD_SENTINEL_TILES_URL;
	static $SERVICES_DASHBOARD_LANDSAT_TILES_URL;
	
	static function init() {
		self::$SERVICES_DASHBOARD_PRODUCTS_URL = self::$SERVICES_BASE_URL.'/GetDashboardProducts';	
		self::$SERVICES_DASHBOARD_SENTINEL_TILES_URL = self::$SERVICES_BASE_URL.'/GetDashboardSentinelTiles'; 
		self::$SERVICES_DASHBOARD_LANDSAT_TILES_URL = self::$SERVICES_BASE_URL.'/GetDashboardLandsatTiles';
		
		// set siteId parameter
		session_start();
		if (isset($_SESSION['siteId']) && !empty($_SESSION['siteId'])) {
			self::$SERVICES_DASHBOARD_PRODUCTS_URL = self::$SERVICES_BASE_URL.'/GetDashboardProducts?siteId='.$_SESSION['siteId'];
		}
	}
}

ConfigParams::init();

?>
