<?php
class ConfigParams {
    static $CONN_STRING  = 'host=localhost port=5432 dbname=sen2agri user=admin password=sen2agri';
    static $SERVICES_URL = 'http://localhost:8080/dashboard';
    static $REST_SERVICES_URL = 'http://localhost:8080';
    static $SITE_ID;
    static $USER_NAME;

    static function init() {
        // set login information
        if (isset($_SESSION['siteId']) && isset($_SESSION['userId']) && isset($_SESSION['userName'])) {
            self::$SITE_ID   = $_SESSION['siteId'];
            self::$USER_NAME = $_SESSION['userName'];
        }
    }
    
    static function getServicePort(){
    	if( file_exists("/usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties")){
		    $lines = file("/usr/share/sen2agri/sen2agri-services/config/sen2agri-services.properties");
		    $searchword = 'server.port';
		    $matches = array();
		    foreach($lines as $k=>$v) {
		    	if(preg_match("/\b$searchword\b/i", $v)) {
		    		$matches = $v;
		    		break;
		    	}
		    }
		    $REST_SERVICES_PORT = trim(substr($matches, strpos($matches,'=')+1));
            self::$REST_SERVICES_URL = 'http://localhost:' . $REST_SERVICES_PORT;
	    }
    }
}

ConfigParams::init();
ConfigParams::getServicePort();
?>
