//This file contains the javascript processing functions that update the page on the client side.

//Update current jobs and server resources --------------------------------------------------------------------------------------------------------------------------

function update_current_jobs(json_data)
{
	//Remove the old rows
	$("#pnl_current_jobs table:first tr.to_be_refreshed").remove();

	json_data.current_jobs.forEach(function(job) {

		var action_buttons = "<div class=\"btn-group\">";
		job.actions.forEach(function(action)
				{
			switch(action) {
			case 1:
				action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\">Pause</button>";
				break;
			case 2:
				action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\">Resume</button>";
				break;
			case 3:
				action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\">Cancel</button>";
				break;
			case 4:
				action_buttons += "<button type=\"button\" class=\"btn btn-default btn-xs\">View Config</button>";
				break;
			} 
				});
		action_buttons += "</div>";

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
		var cpu_history_series = [{data: []}];
		var cpu_history_options = {
				series: {
					lines: {
						show: true,
						fill: true,
						lineWidth: 0.1,
						fillColor: { colors: ["rgba(46, 199, 35, 0.4)", "rgba(46, 199, 35, 0.9)"]}
					}
				},
				xaxis: {
					mode: "time",
					tickSize: [5, "second"],
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
					tickSize: [5, "second"],
					timeformat: ""
				},
				yaxis: {
					min: 0,
					max: server.ram_available,        
					tickSize: server.ram_available / 2,
					tickFormatter: function (v, axis) {
						return v + " GB";
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
					tickSize: [5, "second"],
					timeformat: ""
				},
				yaxis: {
					min: 0,
					max: server.swap_available,        
					tickSize: server.swap_available / 2,
					tickFormatter: function (v, axis) {
						return v + " GB";
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
						return  parseFloat(v).toFixed(2) + " TB";
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
					tickSize: [5, "second"],
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
		$(element_id).html(server.cpu_now + '%');
		element_id = "#server_resources_table_" + counter + "_cpu_history";
		update_plot(element_id, [server.cpu_history], [0]); 

		element_id = "#server_resources_table_" + counter + "_ram";
		$(element_id).html(server.ram_now + ' GB / ' + server.ram_available + ' GB');
		element_id = "#server_resources_table_" + counter + "_ram_history";
		update_plot(element_id, [server.ram_history], [0]); 

		element_id = "#server_resources_table_" + counter + "_swap";
		$(element_id).html(server.swap_now + ' GB / ' + server.swap_available + ' GB');
		element_id = "#server_resources_table_" + counter + "_swap_history";
		update_plot(element_id, [server.swap_history], [0]);

		element_id = "#server_resources_table_" + counter + "_disk";
		$(element_id).html(server.disk_used + " TB / " + server.disk_available + ' TB');
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

function get_system_overview_data()
{
	$.ajax({
		url: get_system_overview_data_url,
        type: "get",
        cache: false,
        crosDomain: true,
        dataType: "json",
		success: function(json_data)
		{
			update_current_jobs(json_data);

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

function set_system_overview_refresh()
{
	// Run the get function now and schedule the next executions.
	get_system_overview_data();
	setInterval(get_system_overview_data, get_system_overview_data_interval);
}

//Update processor statistics --------------------------------------------------------------------------------------------------------------------------

function fill_key_value_table(panel, list)
{
	list.forEach(function(item) {
		var new_row = "<tr class=\"to_be_refreshed\">" +
		"<th>" + item[0] + "</th>" +
		"<td>" + item[1] + "</td>" +
		"</tr>";

		$(panel + " table:first").append(new_row);
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

function get_product_availability_data()
{
	$.ajax({
		url: get_product_availability_data_url,
        type: "get",
        cache: false,
        crosDomain: true,
        dataType: "json",
        data: {
        	since: "2015-07-01T00:00:00"
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
	// Run the get function now and schedule the next executions.
	get_product_availability_data();
	setInterval(get_product_availability_data, get_product_availability_data_interval);
}
