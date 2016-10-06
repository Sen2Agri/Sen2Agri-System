<?php
class ConfigParams {
    static $CONN_STRING  = 'host=localhost port=5432 dbname=sen2agri user=admin password=sen2agri';
    static $SERVICES_URL = 'http://localhost:8080/dashboard';
    static $SITE_ID;
    static $USER_NAME;

    static function init() {
        // set login information
        if (isset($_SESSION['siteId']) && isset($_SESSION['userId']) && isset($_SESSION['userName'])) {
            self::$SITE_ID   = $_SESSION['siteId'];
            self::$USER_NAME = $_SESSION['userName'];
        }
    }
}

ConfigParams::init();
?>
