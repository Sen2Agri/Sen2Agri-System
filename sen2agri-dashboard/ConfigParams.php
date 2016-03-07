<?php
class ConfigParams {
	static $USER_ID;
	static $USER_NAME;
	static $SITE_ID;
	
	static $SERVER_NAME = 'sen2agri-dev';
	static $SERVICES_PORT = '8080';
	static $PRODUCT_ROOT_FOLDER = '/mnt/output';
	
	static $SERVICES_BASE_URL;
	static $WEB_BASE_URL;
	static $SERVICES_DASHBOARD_PRODUCTS_URL;
	static $SERVICES_DASHBOARD_SENTINEL_TILES_URL;
	static $SERVICES_DASHBOARD_LANDSAT_TILES_URL;
	static $SERVICES_NOTIFY_ORCHESTRATOR_URL;
	static $SITE_PRODUCT_RELATIVE_FOLDER;
	
	static function init() {
		self::$USER_ID = $_SESSION['userId'];
		self::$USER_NAME = $_SESSION['userName'];
		self::$SITE_ID = $_SESSION['siteId'];
		
		self::$SERVICES_BASE_URL = 'http://'.self::$SERVER_NAME.':'.self::$SERVICES_PORT.'/dashboard';
		self::$WEB_BASE_URL = 'http://'.self::$SERVER_NAME;
		self::$SITE_PRODUCT_RELATIVE_FOLDER = "http://".self::$SERVER_NAME."/files/Sen2AgriFiles";
		self::$SERVICES_DASHBOARD_PRODUCTS_URL = self::$SERVICES_BASE_URL.'/GetDashboardProducts'.(empty(self::$SITE_ID) ? '' : '?siteId='.self::$SITE_ID);
		self::$SERVICES_DASHBOARD_SENTINEL_TILES_URL = self::$SERVICES_BASE_URL.'/GetDashboardSentinelTiles'; 
		self::$SERVICES_DASHBOARD_LANDSAT_TILES_URL = self::$SERVICES_BASE_URL.'/GetDashboardLandsatTiles';
		self::$SERVICES_NOTIFY_ORCHESTRATOR_URL = self::$SERVICES_BASE_URL.'/NotifyOrchestrator';
	}
}

ConfigParams::init();
?>
