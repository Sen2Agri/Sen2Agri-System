var get_current_job_data_url = "http://sen2agri-dev:8080/dashboard/GetDashboardCurrentJobData";
var get_current_job_data_interval = 60000;

var get_server_resource_data_url = "http://sen2agri-dev:8080/dashboard/GetDashboardServerResourceData";
var get_server_resource_data_interval = 10000;

var get_processor_statistics_url = "http://sen2agri-dev:8080/dashboard/GetDashboardProcessorStatistics";
var get_processor_statistics_interval = 60000;

var get_product_availability_data_url = "http://sen2agri-dev:8080/dashboard/GetDashboardProductAvailability";
var get_product_availability_data_interval = 60000;

var get_job_timeline_url = "http://sen2agri-dev:8080/dashboard/GetDashboardJobTimeline";
var get_job_timeline_interval = 60000;

var pause_job_url = "http://sen2agri-dev:8080/dashboard/PauseJob";

var resume_job_url = "http://sen2agri-dev:8080/dashboard/ResumeJob";

var cancel_job_url = "http://sen2agri-dev:8080/dashboard/CancelJob";

var get_job_config_data_url = "http://sen2agri-dev:8080/dashboard/GetJobConfig";


var services_base_url = "http://sen2agri-dev:8080";
var get_all_sites_url = services_base_url + "/dashboard/GetDashboardSites";
var get_sentinel2_tiles_url = services_base_url + "/dashboard/GetDashboardSentinelTiles";
var get_landsat_tiles_url = services_base_url + "/dashboard/GetDashboardLandsatTiles";
var get_products_url = services_base_url + "/dashboard/GetDashboardProducts";
