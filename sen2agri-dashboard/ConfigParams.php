<?php
class ConfigParams {
	static $USER_ID;
	static $USER_NAME;
	static $SITE_ID;
	static $CONN_STRING;
	
	static $HOST_NAME = 'sen2agri-dev';
	static $DB_NAME = 'sen2agri';
	static $DB_USER = 'admin';
	static $DB_PASS = 'sen2agri';
	static $DB_PORT = '5432';
	
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
		// set login information
		if (isset($_SESSION['siteId']) && isset($_SESSION['userId']) && isset($_SESSION['userName'])) {
			self::$SITE_ID = $_SESSION['siteId'];
			self::$USER_ID = $_SESSION['userId'];
			self::$USER_NAME = $_SESSION['userName'];
		}
		
		// set database connection string
		self::$CONN_STRING = 'host='.self::$HOST_NAME.' port='.self::$DB_PORT.' dbname='.self::$DB_NAME.' user='.self::$DB_USER.' password='.self::$DB_PASS;
		
		// set services urls
		self::$SERVICES_BASE_URL = 'http://'.self::$HOST_NAME.':'.self::$SERVICES_PORT.'/dashboard';
		self::$WEB_BASE_URL = 'http://'.self::$HOST_NAME;
		self::$SITE_PRODUCT_RELATIVE_FOLDER = "http://".self::$HOST_NAME."/files/Sen2AgriFiles";
		self::$SERVICES_DASHBOARD_PRODUCTS_URL = self::$SERVICES_BASE_URL.'/GetDashboardProducts'.(empty(self::$SITE_ID) ? '' : '?siteId='.self::$SITE_ID);
		self::$SERVICES_DASHBOARD_SENTINEL_TILES_URL = self::$SERVICES_BASE_URL.'/GetDashboardSentinelTiles'; 
		self::$SERVICES_DASHBOARD_LANDSAT_TILES_URL = self::$SERVICES_BASE_URL.'/GetDashboardLandsatTiles';
		self::$SERVICES_NOTIFY_ORCHESTRATOR_URL = self::$SERVICES_BASE_URL.'/NotifyOrchestrator';
		
	}
}

ConfigParams::init();
?>
