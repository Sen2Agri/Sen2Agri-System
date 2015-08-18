//This file contains the javascript processing functions that update the page on the client side.

//Update current jobs and server resources --------------------------------------------------------------------------------------------------------------------------

function update_current_jobs(json_data)
{
	//Remove the old rows
	$("#pnl_current_jobs table:first tr.to_be_refreshed").remove();
	
	if(!json_data.current_jobs)
	{
		return;
	}

	json_data.current_jobs.forEach(function(job) {

		var action_buttons = "<div class=\"btn-group\">";
		job.actions.forEach(function(action)
			{
				switch(action) 
				{
					case 1:
						action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action(pause_job_url, " + job.id + ")\">Pause</button>";
						break;
					case 2:
						action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action(resume_job_url, " + job.id + ")\">Resume</button>";
						break;
					case 3:
						action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"perform_job_action(cancel_job_url, " + job.id + ")\">Cancel</button>";
						break;
					case 4:
						action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\" onclick=\"get_job_config(" + job.id + ")\">View Config</button>";
						break;
				} 
			});
		action_buttons += "</div>";
		
		if(!job.current_tasks)
		{	
			// Break if there aren't any tasks; should not happen in 'real life'
			return;
		}

		var new_row = "<tr class=\"to_be_refreshed\">" +
		"<td rowspan=\"" + job.current_tasks.length + "\">" + job.id + "</td>" +
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
		for (idx = 1; idx < job.current_tasks.length; idx++)
		{
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

function update_server_resources_layout(json_data)
{
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
function update_server_resources(json_data)
{
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

function update_plot(element_id, series_data, series_idxs)
{
	var plot = plots[element_id];
	var series = plot.getData();
	var options = plot.getOptions();

	for(i = 0; i < series_idxs.length; i++)
	{
		series[series_idxs[i]].data = series_data[i];
	}	

	plots[element_id] =  $.plot($(element_id), series, options);

}

function get_current_job_data()
{
	$.ajax({
		url: get_current_job_data_url,
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		success: function(json_data)
		{
			update_current_jobs(json_data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
}

function set_current_job_refresh()
{
	// Run the get function now and schedule the next executions.
	get_current_job_data();
	setInterval(get_current_job_data, get_current_job_data_interval);
}

function get_server_resource_data()
{
	$.ajax({
		url: get_server_resource_data_url,
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		success: function(json_data)
		{
			if($("#pnl_server_resources table.to_be_refreshed_when_needed").length != json_data.server_resources.length)
			{
				update_server_resources_layout(json_data);
			}

			update_server_resources(json_data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
}

function set_server_resource_refresh()
{
	// Run the get function now and schedule the next executions.
	get_server_resource_data();
	setInterval(get_server_resource_data, get_server_resource_data_interval);
}

function perform_job_action(action_url, job_id)
{
	$.ajax({
		url: action_url,
		type: "get",
		cache: false,
		crosDomain: true,
		data: {
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
}

function get_job_config(job_id)
{
	$.ajax({
		url: get_job_config_data_url,
		type: "get",
		cache: false,
		crosDomain: true,
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

function show_job_config(json_data)
{
	//Remove the old rows
	$("#popup_content_container table:first tr.to_be_refreshed").remove();
	// First show the configuration parameters
	fill_key_value_table("#popup_content_container", json_data.configuration);
	// Next show the input parameters
	fill_key_value_table("#popup_content_container", json_data.input);
	
	toggleMultipleAdditionalContent([$('#popup_content_parent'), $('#popup_content')], [true, true]);
}

//Update processor statistics --------------------------------------------------------------------------------------------------------------------------

function fill_key_value_table(parent, list)
{
	if (!list)
	{
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

function update_l2a_statistics(json_data)
{
	//Remove the old rows
	$("#pnl_l2a_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l2a_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l2a_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l2a_resources", json_data.l2a_statistics.resources);
	fill_key_value_table("#pnl_l2a_output", json_data.l2a_statistics.output);
	fill_key_value_table("#pnl_l2a_configuration", json_data.l2a_statistics.configuration);
}

function update_l3a_statistics(json_data)
{
	//Remove the old rows
	$("#pnl_l3a_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l3a_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l3a_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l3a_resources", json_data.l3a_statistics.resources);
	fill_key_value_table("#pnl_l3a_output", json_data.l3a_statistics.output);
	fill_key_value_table("#pnl_l3a_configuration", json_data.l3a_statistics.configuration);
}

function update_l3b_statistics(json_data)
{
	//Remove the old rows
	$("#pnl_l3b_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l3b_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l3b_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l3b_resources", json_data.l3b_statistics.resources);
	fill_key_value_table("#pnl_l3b_output", json_data.l3b_statistics.output);
	fill_key_value_table("#pnl_l3b_configuration", json_data.l3b_statistics.configuration);
}

function update_l4a_statistics(json_data)
{
	//Remove the old rows
	$("#pnl_l4a_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l4a_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l4a_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l4a_resources", json_data.l4a_statistics.resources);
	fill_key_value_table("#pnl_l4a_output", json_data.l4a_statistics.output);
	fill_key_value_table("#pnl_l4a_configuration", json_data.l4a_statistics.configuration);
}

function update_l4b_statistics(json_data)
{
	//Remove the old rows
	$("#pnl_l4b_resources table:first tr.to_be_refreshed").remove();
	$("#pnl_l4b_output table:first tr.to_be_refreshed").remove();
	$("#pnl_l4b_configuration table:first tr.to_be_refreshed").remove();

	fill_key_value_table("#pnl_l4b_resources", json_data.l4b_statistics.resources);
	fill_key_value_table("#pnl_l4b_output", json_data.l4b_statistics.output);
	fill_key_value_table("#pnl_l4b_configuration", json_data.l4b_statistics.configuration);
}

function get_processor_statistics()
{
	$.ajax({
		url: get_processor_statistics_url,
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		success: function(json_data)
		{
			update_l2a_statistics(json_data);
			update_l3a_statistics(json_data);
			update_l3b_statistics(json_data);
			update_l4a_statistics(json_data);
			update_l4b_statistics(json_data);
		},
		error: function (responseData, textStatus, errorThrown) {
			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
		}
	});
}

function set_processor_statistics_refresh()
{
	// Run the get function now and schedule the next executions.
	get_processor_statistics();
	setInterval(get_processor_statistics, get_processor_statistics_interval);
}

//Update product availability --------------------------------------------------------------------------------------------------------------------------

function update_product_availability(json_data)
{
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

var product_availability_interval_pointer;
var product_availability_since = (new Date()).toISOString();

function get_product_availability_data()
{
	$.ajax({
		url: get_product_availability_data_url,
		type: "get",
		cache: false,
		crosDomain: true,
		dataType: "json",
		data: {
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

function set_product_availability_data_refresh()
{
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
