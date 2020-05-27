<?php
class ConfigParams {
    static $DEFAULT_DB_HOST = 'localhost';
    static $DEFAULT_DB_PORT = 5432;
    static $DEFAULT_DB_NAME = 'sen2agri';
    static $DEFAULT_DB_USER = 'admin';
    static $DEFAULT_DB_PASS = 'sen2agri';
    static $DEFAULT_SERVICES_URL = 'http://localhost:8082/dashboard';
    static $DEFAULT_REST_SERVICES_URL = 'http://localhost:8080';
    static $DB_NAME;
    static $CONN_STRING;
    static $SERVICES_URL;
    static $REST_SERVICES_URL;
    static $SITE_ID;
    static $USER_NAME;

    public function getConnection(){
        if (!self::$CONN_STRING) {
            $db_host = getenv('DB_HOST') ?: self::$DEFAULT_DB_HOST;
            $db_port = getenv('DB_PORT') ?: self::$DEFAULT_DB_PORT;
            $db_name = getenv('DB_NAME') ?: self::$DEFAULT_DB_NAME;
            $db_user = getenv('DB_USER') ?: self::$DEFAULT_DB_USER;
            $db_pass = getenv('DB_PASS') ?: self::$DEFAULT_DB_PASS;

            self::$DB_NAME = $db_name;
            self::$CONN_STRING = "host=$db_host port=$db_port dbname=$db_name user=$db_user password=$db_pass";
        }

        return self::$CONN_STRING;
    }

    static function init() {
        // set services url and rest services url
        self::$SERVICES_URL = getenv('SERVICES_URL') ?: self::$DEFAULT_SERVICES_URL;
        self::$REST_SERVICES_URL = (getenv('REST_SERVICES_URL') ?: self::getServicesUrl()) ?: self::$DEFAULT_REST_SERVICES_URL;
        self::$DB_NAME = self::$DEFAULT_DB_NAME;
        
		// set login information
        if (isset($_SESSION['siteId']) && isset($_SESSION['userId']) && isset($_SESSION['userName'])) {
            self::$SITE_ID   = $_SESSION['siteId'];
            self::$USER_NAME = $_SESSION['userName'];
        }
    }

    public function isSen2Agri(){
        return self::$DB_NAME == "sen2agri";
    }

    static function getServicesUrl(){
        $props_file = "/usr/share/sen2agri/sen2agri-services/config/services.properties";
        if( file_exists("/usr/share/sen2agri/" . self::$DB_NAME . "-services/config/services.properties")){
            $props_file = "/usr/share/sen2agri/" . self::$DB_NAME . "-services/config/services.properties";
        }
    	if( file_exists($props_file)){
		    $lines = file($props_file);
		    $searchword = 'server.port';
		    $matches = array();
		    foreach($lines as $k=>$v) {
		    	if(preg_match("/\b$searchword\b/i", $v)) {
		    		$matches = $v;
		    		break;
		    	}
		    }
		    $port = trim(substr($matches, strpos($matches,'=')+1));
            return "http://localhost:$port";
        }
        return NULL;
    }
}

ConfigParams::init();
?>
