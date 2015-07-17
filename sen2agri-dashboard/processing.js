// This file contains the javascript processing functions that update the page on the client side.

// Update current jobs --------------------------------------------------------------------------------------------------------------------------

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
		
		var new_row = $("<tr class=\"to_be_refreshed\">" +
							"<td>"+ job.id + "</td>" +
							"<td>"+ job.processor + "</td>" +
							"<td>"+ job.site + "</td>" +
							"<td>"+ job.triggered_by + "</td>" +
							"<td>"+ job.triggered_on + "</td>" +
							"<td>"+ job.status + "</td>" +
							"<td>"+ job.tasks_completed + " / " + job.tasks_remaining + "</td>" +
							"<td>"+ job.current_task_module + "</td>" +
							"<td>"+ job.current_task_steps_completed + " / " + job.current_task_steps_remaining + "</td>" +
							"<td>"+ action_buttons + "</td>" +
						"</tr>");
		
	    $("#pnl_current_jobs table:first tr:last").after(new_row);
	});
	
}

//Update server resources --------------------------------------------------------------------------------------------------------------------------

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
		var cpu_history_series = [{data: null}];
		var cpu_history_options = {
			    series: {
			        lines: {
			            show: true,
			            fill: true,
			            lineWidth: 1.2,
			            fillColor: { colors: ["rgba(108, 164, 213, 0.5)", "rgba(35, 82, 124, 0.5)"]}
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
			    },
			    colors: ["#23527C"]
			};
		
		var element_id = "#server_resources_table_" + counter + "_cpu_history";
		plots[element_id] =  $.plot($(element_id), cpu_history_series, cpu_history_options);
		
		//Add the RAM chart
		var ram_history_series = [{data: null}];
		var ram_history_options = {
			    series: {
			        lines: {
			            show: true,
			            fill: true,
			            lineWidth: 1.2,
			            fillColor: { colors: ["rgba(108, 164, 213, 0.5)", "rgba(35, 82, 124, 0.5)"]}
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
			    },
			    colors: ["#23527C"]
			};
		
		var element_id = "#server_resources_table_" + counter + "_ram_history";
		plots[element_id] =  $.plot($(element_id), ram_history_series, ram_history_options);
		
		//Add the Swap chart
		var swap_history_series = [{data: null}];
		var swap_history_options = {
			    series: {
			        lines: {
			            show: true,
			            fill: true,
			            lineWidth: 1.2,
			            fillColor: { colors: ["rgba(108, 164, 213, 0.5)", "rgba(35, 82, 124, 0.5)"]}
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
			    },
			    colors: ["#23527C"]
			};
		
		var element_id = "#server_resources_table_" + counter + "_swap_history";
		plots[element_id] =  $.plot($(element_id), swap_history_series, swap_history_options);
		
		//Add the Load chart
		var load_history_series = [{data: null}];
		var load_history_options = {
			    series: {
			        lines: {
			            show: true,
			            fill: true,
			            lineWidth: 1.2,
			            fillColor: { colors: ["rgba(108, 164, 213, 0.5)", "rgba(35, 82, 124, 0.5)"]}
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
			    colors: ["#23527C"]
			};
		
		var element_id = "#server_resources_table_" + counter + "_load_history";
		plots[element_id] =  $.plot($(element_id), load_history_series, load_history_options);
		
		counter++;
	});					
}

// This should only be called after update_server_resources_layout.
function update_server_resources(json_data)
{
	var counter = 0; // DO NOT REMOVE - This is used when updating.
	json_data.server_resources.forEach(function(server) {
		
		var element_id = "#server_resources_table_" + counter + "_cpu";
		$(element_id).html(server.cpu_now + '%');
		element_id = "#server_resources_table_" + counter + "_cpu_history";
		update_plot(element_id, server.cpu_history); 
		
		element_id = "#server_resources_table_" + counter + "_ram";
		$(element_id).html(server.ram_now + ' GB / ' + server.ram_available + ' GB');
		element_id = "#server_resources_table_" + counter + "_ram_history";
		update_plot(element_id, server.ram_history); 
		
		element_id = "#server_resources_table_" + counter + "_swap";
		$(element_id).html(server.swap_now + ' GB / ' + server.swap_available + ' GB');
		element_id = "#server_resources_table_" + counter + "_swap_history";
		update_plot(element_id, server.swap_history);
		
		element_id = "#server_resources_table_" + counter + "_disk";
		$(element_id).html(server.disk_used + " TB / " + server.disk_available + ' TB');
		
		element_id = "#server_resources_table_" + counter + "_load";
		$(element_id).html(server.load_1min + " / " + server.load_5min + " / " + server.load_15min);
		element_id = "#server_resources_table_" + counter + "_load_history";
		update_plot(element_id, server.load_history);
		
		counter++;
	});
}

function update_plot(element_id, new_data)
{
	var plot = plots[element_id];
	var series = plot.getData();
	var options = plot.getOptions();
	
	series[0].data = new_data;
	plots[element_id] =  $.plot($(element_id), series, options);
	
}