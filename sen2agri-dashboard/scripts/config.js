var get_current_job_data_url = "http://"+host_name+":"+services_port+"/dashboard/GetDashboardCurrentJobData";
var get_current_job_data_interval = 60000;

var get_server_resource_data_url = "http://"+host_name+":"+services_port+"/dashboard/GetDashboardServerResourceData";
var get_server_resource_data_interval = 10000;

var get_processor_statistics_url = "http://"+host_name+":"+services_port+"/dashboard/GetDashboardProcessorStatistics";
var get_processor_statistics_interval = 60000;

var get_product_availability_data_url = "http://"+host_name+":"+services_port+"/dashboard/GetDashboardProductAvailability";
var get_product_availability_data_interval = 60000;

var get_job_timeline_url = "http://"+host_name+":"+services_port+"/dashboard/GetDashboardJobTimeline";
var get_job_timeline_interval = 60000;

var pause_job_url           = "http://"+host_name+":"+services_port+"/dashboard/PauseJob";
var resume_job_url          = "http://"+host_name+":"+services_port+"/dashboard/ResumeJob";
var cancel_job_url          = "http://"+host_name+":"+services_port+"/dashboard/CancelJob";
var get_job_config_data_url = "http://"+host_name+":"+services_port+"/dashboard/GetJobConfig";

var services_base_url       = "http://"+host_name+":"+services_port;
var get_all_sites_url       = services_base_url + "/dashboard/GetDashboardSites";
var get_sentinel2_tiles_url = services_base_url + "/dashboard/GetDashboardSentinelTiles";
var get_landsat_tiles_url   = services_base_url + "/dashboard/GetDashboardLandsatTiles";
var get_products_url        = services_base_url + "/dashboard/GetDashboardProducts";
var get_processors_url      = services_base_url + "/dashboard/GetDashboardProcessors";
