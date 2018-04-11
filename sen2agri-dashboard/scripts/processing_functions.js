//This file contains the javascript processing functions that update the page on the client side.
var get_current_job_data_interval          = 60000;
var get_server_resource_data_interval      = 10000;
var get_processor_statistics_interval      = 60000;
var get_product_availability_data_interval = 60000;
var get_job_timeline_interval              = 60000;

//Update current jobs and server resources --------------------------------------------------------------------------------------------------------------------------
function update_current_jobs(json_data) {
	//Remove the old rows
	$("#pnl_current_jobs table:first tr.to_be_refreshed").remove();
	
	if(!json_data.current_jobs) {
		return;
	}
	
	json_data.current_jobs.forEach(function(job) {

		var action_buttons = "<div class=\"btn-group\">";
		job.actions.forEach(function(action) {
			switch(action) {
				case 1:
					//action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action(pause_job_url, " + job.id + ")\">Pause</button>";
					action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action('pauseJob', " + job.id + ")\">Pause</button>";
					break;
				case 2:
					//action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action(resume_job_url, " + job.id + ")\">Resume</button>";
					action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action('resumeJob', " + job.id + ")\">Resume</button>";
					break;
				case 3:
					//action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action(cancel_job_url, " + job.id + ")\">Cancel</button>";
					action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action('cancelJob', " + job.id + ")\">Cancel</button>";
					break;
				/*case 4:
					action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"get_job_config(" + job.id + ")\">View Config</button>";
					break;*/
			}
		});
		action_buttons += "</div>";
		
		if(!job.current_tasks) {
			// Break if there aren't any tasks; should not happen in 'real life'
			return;
		}
		
		var new_row = "<tr class=\"to_be_refreshed\">" +
		"<td id='ID' rowspan=\"" + job.current_tasks.length + "\">" + job.id + "</td>" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + job.processor + "</td>" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + job.site + "</td>" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + job.triggered_by + "</td>" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + job.triggered_on + "</td>" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + job.status + "</td>" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + job.tasks_completed + " / " + job.tasks_remaining + "</td>" +
		"<td>" + job.current_tasks[0].current_task_module + "</td>" +
		"<td>" + job.current_tasks[0].current_task_steps_completed + " / " + job.current_tasks[0].current_task_steps_remaining + "</td>" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + action_buttons + "</td>" +
		"</tr>";
		
		$("#pnl_current_jobs table:first").append(new_row);
		
		// For the rest of the current tasks, add their own row
		for (idx = 1; idx < job.current_tasks.length; idx++) {
			new_row = "<tr class=\"to_be_refreshed\">" +
			"<td>" + job.current_tasks[idx].current_task_module + "</td>" +
			"<td>" + job.current_tasks[idx].current_task_steps_completed + " / " + job.current_tasks[idx].current_task_steps_remaining + "</td>" +
			"</tr>";
			
			$("#pnl_current_jobs table:first").append(new_row);
		}
	});
}

//Hashmap to hold the plos objects.
var plots = {};

function update_server_resources_layout(json_data) {
	//Remove the old tables
	$("#pnl_server_resources table.to_be_refreshed_when_needed").remove();

	var counter = 0; // DO NOT REMOVE - This is used when updating.

	json_data.server_resources.forEach(function(server) {

		//Set up the DOM elements
		var table = "<table class=\"table server_resources_table to_be_refreshed_when_needed\">" +
		"<tr>" +
		"<th colspan=\"3\">" + server.name + "</th>" +
		"</tr>" +
		"<tr>" +
		"<th>CPU</th>" +
		"<td id=\"server_resources_table_" + counter + "_cpu\"></td>" +
		"<td>" +
		"<div id=\"server_resources_table_" + counter + "_cpu_history\" class=\"server_resources_table_graph\"></div>" +
		"</td>" +
		"</tr>" +
		"<tr>" +
		"<th>RAM</th>" +
		"<td id=\"server_resources_table_" + counter + "_ram\"></td>" +
		"<td>" +
		"<div id=\"server_resources_table_" + counter + "_ram_history\" class=\"server_resources_table_graph\"></div>" +
		"</td>" +
		"</tr>" +
		"<tr>" +
		"<th>Swap</th>" +
		"<td id=\"server_resources_table_" + counter + "_swap\"></td>" +
		"<td>" +
		"<div id=\"server_resources_table_" + counter + "_swap_history\" class=\"server_resources_table_graph\"></div>" +
		"</td>" +
		"</tr>" +
		"<tr>" +
		"<th>Disk</th>" +
		"<td id=\"server_resources_table_" + counter + "_disk\"></td>" +
		"<td>" +
		"<div id=\"server_resources_table_" + counter + "_disk_percentage\" class=\"server_resources_table_graph\"></div>" +
		"</td>" +
		"</tr>" +
		"<tr>" +
		"<th>Load</th>" +
		"<td id=\"server_resources_table_" + counter + "_load\"></td>" +
		"<td>" +
		"<div id=\"server_resources_table_" + counter + "_load_history\" class=\"server_resources_table_graph\"></div>" +
		"</td>" +
		"</tr>" +
		"<tr>" +
		"<td colspan=\"3\"></td>" + // This adds an extra horizontal line at the bottom
		"</tr>" +
		"</table>";

		$("#pnl_server_resources").append(table);

	    if(window.navigator.userAgent.indexOf ( "MSIE " ) > 0)
	    {
			$("#server_resources_table_" + counter + "_cpu_history").height($("#server_resources_table_0_cpu_history").parent().height() * 90 / 100);
			$("#server_resources_table_" + counter + "_ram_history").height($("#server_resources_table_0_cpu_history").parent().height() * 90 / 100);
			$("#server_resources_table_" + counter + "_swap_history").height($("#server_resources_table_0_cpu_history").parent().height() * 90 / 100);
			$("#server_resources_table_" + counter + "_disk_percentage").height($("#server_resources_table_0_cpu_history").parent().height() * 90 / 100);
			$("#server_resources_table_" + counter + "_load_history").height($("#server_resources_table_0_cpu_history").parent().height() * 90 / 100);
	    }

		//Add the CPU chart
		var cpu_history_series = [{label: "System", data: [], color: "rgba(46, 199, 35, 0.6)"}, {label: "User", data: [], color: "rgba(35, 199, 188, 0.6)"}];
		var cpu_history_options = {
				series: {
					stack: true,
					lines: {
						show: true,
						fill: true,
						lineWidth: 0.1
					}
				},
				xaxis: {
					mode: "time",
					tickSize: [1, "minute"],
					timeformat: ""
				},
				yaxis: {
					min: 0,
					max: 100,
					tickSize: 50,
					tickFormatter: function (v, axis) {
						return v + "%";
					},
					labelWidth: 30
				},
				legend: {
					noColumns: 0,
					position: "nw",
					backgroundOpacity: 0.5,
					margin: [0, 0],
					labelFormatter: function(label, series) {
						return "<span style=\"margin-right: 5px\">" + label + "</span>";
					}
				}
		};

		var element_id = "#server_resources_table_" + counter + "_cpu_history";
		plots[element_id] =  $.plot($(element_id), cpu_history_series, cpu_history_options);

		//Add the RAM chart
		var ram_history_series = [{data: []}];
		var ram_history_options = {
				series: {
					lines: {
						show: true,
						fill: true,
						lineWidth: 0.1,
						fillColor: { colors: ["rgba(156, 35, 199, 0.4)", "rgba(156, 35, 199, 0.9)"]}
					}
				},
				xaxis: {
					mode: "time",
					tickSize: [1, "minute"],
					timeformat: ""
				},
				yaxis: {
					min: 0,
					max: server.ram_available,
					tickSize: server.ram_available / 2,
					tickFormatter: function (v, axis) {
						return  Math.ceil(v) + " " + server.ram_unit;
					},
					labelWidth: 30
				}
		};

		var element_id = "#server_resources_table_" + counter + "_ram_history";
		plots[element_id] =  $.plot($(element_id), ram_history_series, ram_history_options);

		//Add the Swap chart
		var swap_history_series = [{data: []}];
		var swap_history_options = {
				series: {
					lines: {
						show: true,
						fill: true,
						lineWidth: 0.1,
						fillColor: { colors: ["rgba(35, 142, 199, 0.4)", "rgba(35, 142, 199, 0.9)"]}
					}
				},
				xaxis: {
					mode: "time",
					tickSize: [1, "minute"],
					timeformat: ""
				},
				yaxis: {
					min: 0,
					max: server.swap_available,
					tickSize: server.swap_available / 2,
					tickFormatter: function (v, axis) {
						return Math.ceil(v) + " " + server.swap_unit;
					},
					labelWidth: 30
				}
		};

		var element_id = "#server_resources_table_" + counter + "_swap_history";
		plots[element_id] =  $.plot($(element_id), swap_history_series, swap_history_options);

		//Add the Disk chart
		var disk_series = [{data: []}];
		var disk_options = {
				series: {
					bars: {
						show: true,
						lineWidth: 0.1,
						fillColor: "rgba(199, 199, 35, 0.8)"
					}
				},
				bars: {
					align: "center",
					barWidth: 0.5,
					horizontal: true
				},
				xaxis: {
					min: 0,
					max: server.disk_available,
					tickFormatter: function (v, axis) {
						return  Math.ceil(v) + " " + server.disk_unit;
					},
					labelWidth: 40
				},
				yaxis: {
					tickFormatter: function (v, axis) {
						return "";
					},
					labelWidth: 30
				}
		};

		var element_id = "#server_resources_table_" + counter + "_disk_percentage";
		plots[element_id] =  $.plot($(element_id), disk_series, disk_options);

		//Add the Load chart
		var load_history_series = [{label: "1", data: [], color: "rgba(199, 180, 35, 1)"}, {label: "5", data: [], color: "rgba(199, 101, 35, 1)"}, {label: "15", data: [], color: "rgba(199, 35, 35, 1)"}];
		var load_history_options = {
				series: {
					lines: {
						show: true,
						fill: false,
						lineWidth: 2
					}
				},
				xaxis: {
					mode: "time",
					tickSize: [1, "minute"],
					timeformat: ""
				},
				yaxis: {
					min: 0,
					labelWidth: 30
				},
				legend: {
					noColumns: 0,
					position: "nw",
					backgroundOpacity: 0.5,
					margin: [0, 0],
					labelFormatter: function(label, series) {
						return "<span style=\"margin-right: 5px\">" + label + "</span>";
					}
				}
		};

		var element_id = "#server_resources_table_" + counter + "_load_history";
		plots[element_id] =  $.plot($(element_id), load_history_series, load_history_options);

		counter++;
	});
}

//This should only be called after update_server_resources_layout.
function update_server_resources(json_data) {
	var counter = 0; // DO NOT REMOVE - This is used when updating.
	json_data.server_resources.forEach(function(server) {

		var element_id = "#server_resources_table_" + counter + "_cpu";
		$(element_id).html("<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"CPU Used by System\" >" + server.cpu_system_now + " %</span> " +
				"/ <span data-toggle=\"tooltip\" data-placement=\"top\" title=\"CPU Used by User\" >" + server.cpu_user_now + " %</span> ");
		element_id = "#server_resources_table_" + counter + "_cpu_history";
		update_plot(element_id, [server.cpu_system_history, server.cpu_user_history], [0,1]);

		element_id = "#server_resources_table_" + counter + "_ram";
		$(element_id).html("<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"RAM Used\" >" + server.ram_now + " " + server.ram_unit + "</span> / "
				+ "<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"RAM Available\" >" + server.ram_available + " " + server.ram_unit + "</span>");
		element_id = "#server_resources_table_" + counter + "_ram_history";
		update_plot(element_id, [server.ram_history], [0]);

		element_id = "#server_resources_table_" + counter + "_swap";
		$(element_id).html("<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"Swap Used\" >" + server.swap_now + " " + server.swap_unit + "</span> / "
				+ "<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"Swap Available\" >" + server.swap_available + " " + server.swap_unit + "</span>");
		element_id = "#server_resources_table_" + counter + "_swap_history";
		update_plot(element_id, [server.swap_history], [0]);

		element_id = "#server_resources_table_" + counter + "_disk";
		$(element_id).html("<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"Disk Used\" >" + server.disk_used+ " " + server.disk_unit + "</span> / "
				+ "<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"Disk Available\" >" + server.disk_available + " " + server.disk_unit + "</span>");
		element_id = "#server_resources_table_" + counter + "_disk_percentage";
		var disk_series = [[ server.disk_used, 0 ]];
		update_plot(element_id, [disk_series], [0]);

		element_id = "#server_resources_table_" + counter + "_load";
		$(element_id).html("<span data-toggle=\"tooltip\" data-placement=\"top\" title=\"1 min average\" >" + server.load_1min + "</span> " +
				"/ <span data-toggle=\"tooltip\" data-placement=\"top\" title=\"5 min average\" >" + server.load_5min + "</span> " +
				"/ <span data-toggle=\"tooltip\" data-placement=\"top\" title=\"15 min average\" >" + server.load_15min + "</span> ");
		element_id = "#server_resources_table_" + counter + "_load_history";
		update_plot(element_id, [server.load_1min_history, server.load_5min_history, server.load_15min_history], [0,1,2]);

		counter++;
	});

	// Enable the newly added tooltips.
	$('[data-toggle="tooltip"]').tooltip();
}

function update_plot(element_id, series_data, series_idxs) {
	var plot = plots[element_id];
	var series = plot.getData();
	var options = plot.getOptions();

	for(i = 0; i < series_idxs.length; i++)
	{
		series[series_idxs[i]].data = series_data[i];
	}

	plots[element_id] =  $.plot($(element_id), series, options);

}

function move_to_first_jobs_page() {
	jsonJobsPage = 1;
	get_current_job_data();
}
function move_to_previous_jobs_page() {
	if (jsonJobsPage > 1) {
		jsonJobsPage --;
		get_current_job_data();
	} else {
		jsonJobsPage = 1;
	}
}
function move_to_next_jobs_page() {
	if (!$("#page_move_next").hasClass("disabled")) {
		jsonJobsPage ++;
		get_current_job_data();
	}
}

function get_current_job_data() {
	$.ajax({
		//url: get_current_job_data_url + "?page=" + jsonJobsPage,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			action: "getDashboardCurrentJobData",
			page: jsonJobsPage
		},
		success: function(json_data)
		{
			update_current_jobs(json_data);
			
			// display page number
			$("#page_current").html(jsonJobsPage);
            if (jsonJobsPage == 1) {
                $("#page_move_prev").addClass("disabled");
                $("#page_move_first").addClass("disabled");
            } else {
                $("#page_move_prev").removeClass("disabled");
                $("#page_move_first").removeClass("disabled");
            }            
			// toggle move_next button availability
			if ($("#pnl_current_jobs #ID").length < 5) {
				$("#page_move_next").addClass("disabled");
			} else {
				$("#page_move_next").removeClass("disabled");
			}
			
			// Schedule the next request
			setTimeout(get_current_job_data, get_current_job_data_interval);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			// Schedule the next request
			setTimeout(get_current_job_data, get_current_job_data_interval);
		}
	});
}
function set_current_job_refresh() {
	// Run the get function now and schedule the next executions.
	get_current_job_data();
	//setInterval(get_current_job_data, get_current_job_data_interval);
}

function get_server_resource_data() {
	$.ajax({
		//url: get_server_resource_data_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			action: "getDashboardServerResourceData"
		},
		success: function(json_data)
		{
			if($("#pnl_server_resources table.to_be_refreshed_when_needed").length != json_data.server_resources.length)
			{
				update_server_resources_layout(json_data);
			}

			update_server_resources(json_data);
			// Schedule the next request
			setTimeout(get_server_resource_data, get_server_resource_data_interval);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			// Schedule the next request
			setTimeout(get_server_resource_data, get_server_resource_data_interval);
		}
	});
}
function set_server_resource_refresh() {
	// Run the get function now and schedule the next executions.
	get_server_resource_data();
	//setInterval(get_server_resource_data, get_server_resource_data_interval);
}


function perform_job_action(action_url, job_id) {
	$.ajax({
		//url: action_url,
		url: "processing.php",
		//type: "get",
		type: "post",
		cache: false,
		crosDomain: true,
		data: {
			action: action_url,
			jobId: job_id
		},
		success: function()
		{
			get_current_job_data();
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
}/*
function get_job_config(job_id) {
	$.ajax({
		url: get_job_config_data_url,
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			jobId: job_id
		},
		success: function(json_data)
		{
			show_job_config(json_data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
}
function show_job_config(json_data) {
	//Remove the old rows
	$("#popup_content_container table:first tr.to_be_refreshed").remove();
	// First show the configuration parameters
	fill_key_value_table("#popup_content_container", json_data.configuration);
	// Next show the input parameters
	fill_key_value_table("#popup_content_container", json_data.input);

	toggleMultipleAdditionalContent([$('#popup_content_parent'), $('#popup_content')], [true, true]);
}
*/

//Update processor statistics --------------------------------------------------------------------------------------------------------------------------
function fill_key_value_table(parent, list) {
	if (!list) {
		return;
	}

	list.forEach(function(item) {
		var new_row = "<tr class=\"to_be_refreshed\">" +
		"<th>" + item[0] + "</th>" +
		"<td>" + item[1] + "</td>" +
		"</tr>";

		$(parent + " table:first").append(new_row);
	});
}

function update_l2a_statistics(json_data) {
	//Remove the old rows
	$("#pnl_l2a_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l2a_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l2a_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l2a_resources", json_data.l2a_statistics.resources);
	fill_key_value_table("#pnl_l2a_output", json_data.l2a_statistics.output);
	fill_key_value_table("#pnl_l2a_configuration", json_data.l2a_statistics.configuration);
}
function update_l3a_statistics(json_data) {
	//Remove the old rows
	$("#pnl_l3a_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l3a_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l3a_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l3a_resources", json_data.l3a_statistics.resources);
	fill_key_value_table("#pnl_l3a_output", json_data.l3a_statistics.output);
	fill_key_value_table("#pnl_l3a_configuration", json_data.l3a_statistics.configuration);
}
function update_l3b_statistics(json_data) {
	//Remove the old rows
	$("#pnl_l3b_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l3b_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l3b_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l3b_resources", json_data.l3b_lai_statistics.resources);
	fill_key_value_table("#pnl_l3b_output", json_data.l3b_lai_statistics.output);
	fill_key_value_table("#pnl_l3b_configuration", json_data.l3b_lai_statistics.configuration);
}
function update_l3e_pheno_statistics(json_data)  {
 	//Remove the old rows
 	$("#pnl_l3e_pheno_resources table:first tr.to_be_refreshed").remove();
 	$("#pnl_l3e_pheno_output table:first tr.to_be_refreshed").remove();
 	$("#pnl_l3e_pheno_configuration table:first tr.to_be_refreshed").remove();

 	fill_key_value_table("#pnl_l3e_pheno_resources", json_data.l3e_pheno_statistics.resources);
 	fill_key_value_table("#pnl_l3e_pheno_output", json_data.l3e_pheno_statistics.output);
 	fill_key_value_table("#pnl_l3e_pheno_configuration", json_data.l3e_pheno_statistics.configuration);
}
function update_l4a_statistics(json_data) {
	//Remove the old rows
	$("#pnl_l4a_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l4a_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l4a_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l4a_resources", json_data.l4a_statistics.resources);
	fill_key_value_table("#pnl_l4a_output", json_data.l4a_statistics.output);
	fill_key_value_table("#pnl_l4a_configuration", json_data.l4a_statistics.configuration);
}
function update_l4b_statistics(json_data) {
	//Remove the old rows
	$("#pnl_l4b_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l4b_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l4b_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l4b_resources", json_data.l4b_statistics.resources);
	fill_key_value_table("#pnl_l4b_output", json_data.l4b_statistics.output);
	fill_key_value_table("#pnl_l4b_configuration", json_data.l4b_statistics.configuration);
}

function get_processor_statistics() {
	$.ajax({
		//url: get_processor_statistics_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			action: "getDashboardProcessorStatistics"
		},
		success: function(json_data)
		{
			update_l2a_statistics(json_data);
			update_l3a_statistics(json_data);
			update_l3b_statistics(json_data);
			update_l3e_pheno_statistics(json_data);
			update_l4a_statistics(json_data);
			update_l4b_statistics(json_data);
			
			// Schedule the next request
			setTimeout(get_processor_statistics, get_processor_statistics_interval);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			// Schedule the next request
			setTimeout(get_processor_statistics, get_processor_statistics_interval);
		}
	});
}
function set_processor_statistics_refresh() {
	// Run the get function now and schedule the next executions.
	get_processor_statistics();
	//setInterval(get_processor_statistics, get_processor_statistics_interval);
}

//Update product availability --------------------------------------------------------------------------------------------------------------------------
var product_availability_interval_pointer;
var product_availability_since = (new Date()).toISOString();
function update_product_availability(json_data) {
	nv.addGraph(function() {

		var tree = nv.models.indentedTree()
		.tableClass('table table-striped') //for bootstrap styling
		.columns([
		          {
		        	  key: 'key',
		        	  /*label: 'Product',*/
		        	  showCount: true,
		        	  width: '50%',
		        	  type: 'text'
		          },
		          {
		        	  key: 'info',
		        	  /*label: 'Info',*/
		        	  width: '50%',
		        	  type: 'text'
		          }
		          ]);


		d3.select('#products_availability_tree')
		.datum(json_data.products)
		.call(tree);

		return tree;
	});
}
function get_product_availability_data() {
	$.ajax({
		//url: get_product_availability_data_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			action: "getDashboardProductAvailability",
			since: product_availability_since
		},
		success: function(json_data)
		{
			update_product_availability(json_data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
}
function set_product_availability_data_refresh() {
	$("#cbo_products_since li a").click(function(){
			  var selText = $(this).text();
			  $("#cbo_products_since_selection").html(selText+'<span class="caret"></span>');

			  var since = new Date();

			  switch(this.id)
			  {
			  	case "products_since_1_hour":
			  		since.setHours(since.getHours() - 1);
			  		break;
			  	case "products_since_6_hours":
			  		since.setHours(since.getHours() - 6);
			  		break;
			  	case "products_since_12_hours":
			  		since.setHours(since.getHours() - 12);
			  		break;
		  		case "products_since_1_day":
		  			since.setDate(since.getDate() - 1);
			  		break;
		  		case "products_since_7_days":
		  			since.setDate(since.getDate() - 7);
			  		break;
		  		case "products_since_14_days":
		  			since.setDate(since.getDate() - 14);
			  		break;
		  		case "products_since_30_days":
		  			since.setDate(since.getDate() - 30);
			  		break;
			  }

			  product_availability_since = since.toISOString();

			  // Clear the old schedules, run the get function now and schedule the next executions.
			  clearInterval(product_availability_interval_pointer);
    		  get_product_availability_data();
			  product_availability_interval_pointer = setInterval(get_product_availability_data, get_product_availability_data_interval);
		});

	// Run the get function now and schedule the next executions.
	get_product_availability_data();
	product_availability_interval_pointer = setInterval(get_product_availability_data, get_product_availability_data_interval);
}

function get_job_timeline(jobId) {
	$.ajax({
		//url: get_job_timeline_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			action: "getDashboardJobTimeline",
            jobId: jobId
		},
		success: function(json_data)
		{
			update_job_timeline(json_data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
}
function update_job_timeline(json_data) {
    var container = $("#visualization").get(0);
    var timeline = new vis.Timeline(container, json_data.items, json_data.groups);
}

String.prototype.rtrim = function (s) {
	    if (s == undefined)
	        s = '\\s';
	    return this.replace(new RegExp("[" + s + "]*$"), '');
	};

function add_events(){
	$("button[name='btnFilter']").attr('disabled',true);
	
	//add event on all elements select season
	var selSeasons = $("select#choose_season");
	$.each(selSeasons, function(index, selSeason) {
		$(selSeason).on('change',function(event){	
			var formId = event.target.form.id;
			if($(this).val()!=""){
				$("#"+formId+" input[name='enddate']").attr('disabled',true);
				$("#"+formId+" input[name='startdate']").attr('disabled',true);			
			}else{
				$("#"+formId+" input[name='enddate']").removeAttr('disabled');
				$("#"+formId+" input[name='startdate']").removeAttr('disabled');
				}
		});
	});

	//add event on interval elements
	var endDates = $("input[name='enddate']");
	$.each(endDates, function(index, endDate) {		
		$(endDate).on('change',function(event){		
			if($(this).val()=="" && $("#"+event.target.form.id+" input[name='startdate']").val()==""){
				$("#"+event.target.form.id+" select[name='choose_season']").removeAttr('disabled');							
			}else{
				$("#"+event.target.form.id+" select[name='choose_season']").attr('disabled',true);			
				}
		});
	});

	var startDates = $("input[name='startdate']");
	$.each(startDates, function(index, startDate) {		
		$(startDate).on('change',function(event){		
			if($(this).val()=="" && $("#"+event.target.form.id+" input[name='enddate']").val()==""){
				$("#"+event.target.form.id+" select[name='choose_season']").removeAttr('disabled');									
			}else{
				$("#"+event.target.form.id+" select[name='choose_season']").attr('disabled',true);	
				}
		});
	});
	
	var resetFilters = $("button[name='btnResetFilter']");
	$.each(resetFilters, function(index, resetFilter) {		
		$(resetFilter).on('click',function(event){
			var formId = event.target.form.id;
			$(this).attr('disabled',true);
			
			get_products($("#"+formId+" select#siteId").val(), $("#"+formId+" select#inputFiles"),formId);
			
			get_tiles($("#"+formId+" select#siteId").val(), formId);
			
			$("#"+formId+" select[name='choose_season']").prop('selectedIndex',0);
			$("#"+formId+" select[name='choose_season']").removeAttr('disabled');	
			
			$("#"+formId+" input[name='startdate']").val('').datepicker({  clearBtn: true});
			$("#"+formId+" input[name='enddate']").val('').datepicker({  clearBtn: true});
			$("#"+formId+" input[name='enddate']").datepicker("option",{"minDate":null});
			$("#"+formId+" input[name='startdate']").datepicker("option",{"maxDate":null});
			
			$("#"+formId+" input[name='enddate']").removeAttr('disabled');	
			$("#"+formId+" input[name='startdate']").removeAttr('disabled');	
			
			
			//delete error messages for tiles
			$("#"+formId+" .invalidTilesL8").text("");
			$("#"+formId+" .invalidTilesS2").text("");
			
		});
	});
}

function filter_input_files(formId){
	var tilesL8NotValid = '';var tilesS2NotValid = '';

	var siteId = $("#"+formId+" select#siteId").val();
	var productEl = $("#"+formId+" select#inputFiles");
	
	var data = {};
	var season_id = $("#"+formId+" select#choose_season").find(":selected").val();
	if(season_id!="") data.season_id= season_id ;

	var start_data = $("#"+formId+" input[name='startdate']").val();
	if(start_data!="") data.start_data= start_data ;

	var end_data =  $("#"+formId+" input[name='enddate']").val();
	if(end_data!="") data.end_data= end_data ;
	
	var tilesL8Arr=new Array();
	if($("#"+formId+" input#l3a_chkL8").is(':checked')){
		//check if the tiles from textarea have the good format
		var tilesL8 = $("#"+formId+" textarea[name='L8Tiles']").val();

		if(tilesL8!=""){
			tilesL8Arr = tilesL8.split(',');
			for(i = 0; i < tilesL8Arr.length; i++){
				var noMatch = tilesL8Arr[i].replace(/^(\d{6})(,\d{6})*/gm,'');
				if(noMatch.trim()!=''){
					tilesL8NotValid = tilesL8NotValid+" "+ tilesL8Arr[i];
				}
			}
		}
		
	}
	if(tilesL8NotValid!=''){
		$("#"+formId+" .invalidTilesL8").text("Invalid tile: "+tilesL8NotValid);
	}else{
		$("#"+formId+" .invalidTilesL8").text("");
		}
	
	var tilesS2Arr=new Array();
	if($("#"+formId+" input#l3a_chkS2").is(':checked')){
		//check if the tiles from textarea have the good format 
		var tilesS2 = $("#"+formId+" textarea[name='S2Tiles']").val();
		
		if(tilesS2!=""){
			tilesS2Arr = tilesS2.split(',');
			for(i = 0; i < tilesS2Arr.length; i++){
				var noMatch = tilesS2Arr[i].replace(/^(\d{2}[A-Z]{3})(,\d{2}[A-Z]{3})*/gm,'');
				if(noMatch.trim()!=''){
					tilesS2NotValid = tilesS2NotValid+" "+tilesS2Arr[i];
				}
			}
		}
		
	}
	if(tilesS2NotValid!=''){
		$("#"+formId+" .invalidTilesS2").text("Invalid tile: "+tilesS2NotValid);
	}else{
		$("#"+formId+" .invalidTilesS2").text("");
		}

	data.tiles= tilesL8Arr.concat(tilesS2Arr);
	
	if(tilesS2NotValid=='' && tilesL8NotValid==''){
		//make available the button to reset the filter
		$("#"+formId+" button[name='btnResetFilter']").prop('disabled',false);
		
		get_products(siteId, productEl, formId, data);
	}
	
}

function get_tiles(siteId, formId){
	$.ajax({
		url: "processing.php",
		type: "GET",
		crossDomain: true,
		dataType: "json",
		data: {action: 'getTiles', siteId:siteId, satelliteId:'2'},
		success:function(jsonData){
			$('#'+formId+' textarea[name="L8Tiles"]').val(jsonData);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			update_sites(new Array());
		}
	});
	
	$.ajax({
		url: "processing.php",
		type: "GET",
		crossDomain: true,
		dataType: "json",
		data: {action: 'getTiles', siteId:siteId, satelliteId:'1'},
		success:function(jsonData){
			$('#'+formId+' textarea[name="S2Tiles"]').val(jsonData);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			update_sites(new Array());
		}
	});
}

function seasons_for_site(seasonEl, site_id){
	$(seasonEl).find('option').not(':first').remove();
	
	//get seasons for selected site
	$.ajax({
		type: "POST",
        url: "getTiles.php",
        dataType: "json",	                   
        data: {action: 'get_site_seasons',site_id:site_id},
        success: function(data){                   	
            	
        	for(var i=0;i<data.length;i++){	    
            	var option = $("<option></option>");                		
		           option.val(data[i][0]);
		           option.text(data[i][1]);
		            
		           $(seasonEl).append(option);		
            }
           }
		});
}

function initialise_datepickers(){
	var startdateAll = $('input[name="startdate"]');
	$.each(startdateAll, function(index, startdateEl) {
	//$('input[name="startdate"]').each(function(){
		$(startdateEl).datepicker({
			dateFormat: "yy-mm-dd",
			onSelect: function(selected) {
			    $('#'+this.form.id+' input[name="enddate"]').datepicker("option","minDate", selected);
			    $('#'+this.form.id+' select[name="choose_season"]').attr('disabled',true);	
			        }
		});

	});
	
	$('input[name="enddate"]').each(function(){
		$(this).datepicker({
			dateFormat: "yy-mm-dd",
			onSelect: function(selected) {
				    $('#'+this.form.id+' input[name="startdate"]').datepicker("option","maxDate", selected);
				    $('#'+this.form.id+' select[name="choose_season"]').attr('disabled',true);	
				        }});
	});
}

function get_all_sites() {
	$.ajax({
		//url: get_all_sites_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
            action: "getDashboardSites"
		},
		success: function(json_data)
		{
			update_sites(json_data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			update_sites(new Array());
		}
	});
}
function update_sites(json_data) {
	var siteElAll = $("select#siteId");
	$.each(siteElAll, function(index, siteEl) {
		//Remove the old options
		siteEl = $(siteEl);
		siteEl.empty();
		siteEl.append('<option value="">Select a site</option>');
		$.each(json_data, function(index, siteObj) {
			if ((jsonSiteId) == 0 || (jsonSiteId.indexOf(siteObj.id.toString())>-1)) {
				siteEl.append('<option value="'+siteObj.id+'">'+siteObj.name+'</option>');
			};
		});
		
		siteEl.change(function (event) {
			var siteEl = event.target;
			//var sentinel2TilesEl = $("#"+siteEl.form.id+" select#sentinel2Tiles");
			//var landsatTilesEl = $("#"+siteEl.form.id+" select#landsatTiles");
			var productsEl = $("#"+siteEl.form.id+" select#inputFiles");
			var cropMaskEl = $("#"+siteEl.form.id+" select#cropMask");
			var seasonEl =  $("#"+siteEl.form.id+" select#choose_season");
			var btnFilterEl =  $("#"+siteEl.form.id+" button[name='btnFilter']");
		
			if(siteEl.selectedIndex > 0) {
				//get_sentinel2_tiles(siteEl.options[siteEl.selectedIndex].value, sentinel2TilesEl);
				//get_landsat_tiles(siteEl.options[siteEl.selectedIndex].value, landsatTilesEl);
				get_products(siteEl.options[siteEl.selectedIndex].value, productsEl, siteEl.form.id);
				get_crop_mask(siteEl.options[siteEl.selectedIndex].value, cropMaskEl);
				
				//get seasons for the selected site
				$(seasonEl).removeAttr("disabled");
				seasons_for_site(seasonEl,siteEl.options[siteEl.selectedIndex].value);
				
				//get tiles for the selected site
				get_tiles(siteEl.options[siteEl.selectedIndex].value,siteEl.form.id);
				$("#"+siteEl.form.id+" .invalidTilesL8").text('');
				$("#"+siteEl.form.id+" .invalidTilesS2").text('');
				
				$(btnFilterEl).removeAttr('disabled');
			} else {
				//update_sentinel2_tiles(new Array(), sentinel2TilesEl);
				//update_landsat_tiles(new Array(), landsatTilesEl);
				update_products(new Array(), productsEl);
				update_crop_mask(new Array(), cropMaskEl);
				
				$(btnFilterEl).attr('disabled',true);
				$(seasonEl).attr('disabled',true);
			}
		});
	});
	
	var chkL8All = $(".chkL8");
	$.each(chkL8All, function(index, chkL8) {
		chkL8 = $(chkL8);
		chkL8.change(function (event) {
			var chkEl = event.target;
			var siteEl     = $("#"+chkEl.form.id+" select#siteId")[0];
			var productsEl = $("#"+chkEl.form.id+" select#inputFiles");
			var cropMaskEl = $("#"+chkEl.form.id+" select#cropMask");
			if(siteEl.selectedIndex > 0) {
				var siteId = siteEl.options[siteEl.selectedIndex].value;
				//get_products(siteId, productsEl, chkEl.form.id);
				get_crop_mask(siteId, cropMaskEl);
			} else {
				//update_products(new Array(), productsEl);
				update_crop_mask(new Array(), cropMaskEl);
			}
			
			var value = chkEl.value;
			if(chkEl.checked){
				$("#"+chkEl.form.id+" textarea[name='"+value+"Tiles']").prop('disabled', false);
			}else{
				$("#"+chkEl.form.id+" textarea[name='"+value+"Tiles']").prop('disabled', true);
			}

		});
	});
	var chkS2All = $(".chkS2");
	$.each(chkS2All, function(index, chkS2) {
		chkS2 = $(chkS2);
		chkS2.change(function (event) {
			var chkEl = event.target;
			var siteEl     = $("#"+chkEl.form.id+" select#siteId")[0];
			var productsEl = $("#"+chkEl.form.id+" select#inputFiles");
			var cropMaskEl = $("#"+chkEl.form.id+" select#cropMask");
			if(siteEl.selectedIndex > 0) {
				//get_products(siteEl.options[siteEl.selectedIndex].value, productsEl, chkEl.form.id);
				get_crop_mask(siteEl.options[siteEl.selectedIndex].value, cropMaskEl);
			} else {
				//update_products(new Array(), productsEl);
				update_crop_mask(new Array(), cropMaskEl);
			}
		});
	});
}

function get_crop_mask(siteId, cropMaskEl) {
	$.ajax({
		//url: get_products_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			action: "getDashboardProducts",
			siteId: siteId,
			processorId: l4a_proc_id
		},
		success: function(json_data)
		{
			update_crop_mask(json_data, cropMaskEl);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			update_crop_mask(new Array(), cropMaskEl);
		}
	});
}
function update_crop_mask(json_data, cropMaskEl) {
	//Remove the old options
	cropMaskEl.empty();
	
	cropMaskEl.append('<option value="">Select a tile</option>');
	
	$.each(json_data, function(index, productObj) {
		cropMaskEl.append('<option value="'+productObj.product+'">'+productObj.product+'</option>');
	});
}

function get_products(siteId, productsEl, formId, filter) {
	var data = {action: "getDashboardProducts",	siteId: siteId,	processorId: l2a_proc_id};
	if(filter !== undefined){
		data = Object.assign({}, data, filter);
	}
	if($("#"+formId+" input#l3a_chkS2").is(':checked') && !$("#"+formId+" input#l3a_chkL8").is(':checked')){
		if(data.tiles==null){
			data.satellite_id = 1;
		}		
	}else if(!$("#"+formId+" input#l3a_chkL8").is(':checked') && $("#"+formId+" input#l3a_chkS2").is(':checked')){
		if(data.tiles==null){
			data.satellite_id = 2;
		}
	}
	
	$.ajax({
		//url: get_products_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: data,
		success: function(json_data)
		{
			update_products(json_data, productsEl);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			update_products(new Array(), productsEl);
		}
	});
}
function update_products(json_data, productsEl) {
	//Remove the old options
	productsEl.empty();
	
	$.each(json_data, function(index, productObj) {
		// select S2 products
		var chkS2 = $("#"+productsEl[0].form.id+" .chkS2");
		if (((chkS2.length == 0) || chkS2.is(":checked")) && productObj.satellite_id == 1) {
			productsEl.append('<option value="'+productObj.product+'">'+productObj.product+'</option>');	
		}
		// select L8 products
		var chkL8 = $("#"+productsEl[0].form.id+" .chkL8");
		if (((chkL8.length == 0) || chkL8.is(":checked")) && productObj.satellite_id == 2) {
			productsEl.append('<option value="'+productObj.product+'">'+productObj.product+'</option>');	
		}
	});
}

function get_processor_id(proc_short_name, assign_to) {
	$.ajax({
		//url: get_processors_url,
		url: "processing.php",
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
			action: "getDashboardProcessors"
		},
		success: function(json_data)
		{
			var found = false;
			$.each(json_data, function(index, processorObj) {
				if(processorObj.short_name == proc_short_name) { 
					eval(assign_to + ' = ' + processorObj.id);
					found = true;
					return;
				}
			});
			if(!found) eval(assign_to + ' = null;');
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
			eval(assign_to + ' = null;');
		}
	});
}
