<?php
class ConfigParams {
    static $DB_NAME = 'sen2agri';
    static $CONN_STRING;
    static $SERVICES_URL = 'http://localhost:8080/dashboard';
    static $REST_SERVICES_URL = 'http://localhost:8080';
    static $SITE_ID;
    static $USER_NAME;

    public function __construct(){
        self::setConnection();
    }
    
    public function getConnection(){
        self::setConnection();
        return self::$CONN_STRING;
    }
    
    private static function setConnection(){
        if(!isset(self::$CONN_STRING)){
            self::$CONN_STRING = 'host=localhost port=5432 dbname='.self::$DB_NAME.' user=admin password=sen2agri';
        }
    }
    
    static function init() {
        // set login information
        if (isset($_SESSION['siteId']) && isset($_SESSION['userId']) && isset($_SESSION['userName'])) {
            self::$SITE_ID   = $_SESSION['siteId'];
            self::$USER_NAME = $_SESSION['userName'];
        }
    }
    
    public function isSen2Agri(){
        if(self::$DB_NAME == "sen2agri")
            return true;
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
