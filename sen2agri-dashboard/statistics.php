<?php 

require_once("ConfigParams.php");
include 'master.php';

function select_site() {
    $db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
	if ($_SESSION['isAdmin']) {
		// admin
		$sql    = "SELECT * FROM sp_get_sites()";
		$result = pg_query ( $db, $sql ) or die ( "Could not execute." );
	} else {
		// not admin
		$sql    = "SELECT * FROM sp_get_sites($1)";
		$result = pg_query_params ( $db, $sql, array ("{".implode(',',$_SESSION ['siteId'])."}") ) or die ( "Could not execute." );
	}
	$option_site = "<option value=\"0\" selected>Select a site...</option>";
	while ( $row = pg_fetch_row ( $result ) ) {
		$option_site .= "<option value=\"$row[0]\">" . $row [1] . "</option>";
	}
	echo $option_site;
}
function select_satellite() {
	$dbconn = pg_connect(ConfigParams::getConnection()) or die ("Could not connect");
	$rows = pg_query($dbconn, "SELECT id, INITCAP(satellite_name) as satellite_name FROM satellite ORDER BY satellite_name") or die ("Could not execute.");
	$res =  pg_fetch_all($rows);
	$option_sat = "<option value=\"0\" selected>Select satellite...</option>";
	foreach ($res as $sat){
		$val = "";
		switch ($sat['satellite_name']) {
			case "Sentinel1": $val = "S1"; break;
			case "Sentinel2": $val = "S2"; break;
			case "Landsat8" : $val = "L8"; break;
		}
		$option_sat .= "<option value='" . $val . "'>" . $sat['satellite_name'] . "</option>";
	}
	echo $option_sat;
}
?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<!-------------- Orbit statistics ---------------->
			<style>
				.nvtooltip{position:fixed !important}
			
				#accordion_statistics *{box-sizing:border-box}
				#accordion_statistics .panel-body{border:0}
				.statistics .form-group-sm{margin-bottom:5px}
				.statistics .form-group-sm .form-control{height:30px}
				.statistics .form-group-sm .control-label{font-size:14px;line-height:26px}
				.statistics .form-group-sm .radio-group{font-size:14px}
				.statistics .form-group-sm .radio-group input{width:30px}
				.statistics .form-group-sm .radio-group label{width:100%;font-weight:400;margin:0}
				.statistics .filters{background-color:#edf2e4;padding:10px;border:1px solid #dddddd;border-radius:5px}
				.statistics .reports-orbit,.statistics .reports-aggregate{padding:10px;text-align:center}
				.statistics .reports-aggregate{background:transparent url(images/chart.png) no-repeat 50% 50%;min-height:600px}
				.statistics .reports-orbit    {background:transparent url(images/radar.png) no-repeat 50% 50%;min-height:600px}
				.statistics input.add-edit-btn{width:100%;border-radius:4px;font-size:14px;text-align:center;margin-top:10px}
				.statistics h4.panel-title a, .statistics h4.panel-title span{width:100%}
				
				#charts_aggregate .chart-container:first-child{margin-top:100px}
				#charts_aggregate .chart-container:first-child svg{overflow:visible}
				.nvd3 g.nv-groups path.nv-line{stroke-width:3px}
				.nvd3 g.nv-y2 .nv-axislabel{font:15px Arial,Helvetica,sans-serif;font-weight:700;fill:#336cc1}
				.chart-title{color:#81a53e;font-weight:700}
				.chart-title>span{color:#336cc1}
				.chart-title~svg{margin-bottom:20px;height:300px;width:100%}
			</style>
			<div class="container-fluid">
				<div class="panel-group statistics config" id="accordion_statistics">
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion_statistics" href="#reports_aggregate" class="collapsed" aria-expanded="false">Monthly Preprocessing Reports</a>
							</h4>
						</div>						
						<div id="reports_aggregate" class="panel-collapse collapse">
							<div class="panel-body">
								<form id="filters_aggregate" role="form" method="post">
									<div class="col-md-3 filters">
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="site_select">Site:</label>
											<div class="col-md-9">
												<select class="form-control" name="site_select"><?php select_site();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="satellite_select">Satellite:</label>
											<div class="col-md-9">
												<select class="form-control" name="satellite_select"><?php select_satellite();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="orbit_select">Orbit:</label>
											<div class="col-md-9">
												<select size="6" class="form-control" name="orbit_select" style="height:auto">
													<option value="0" selected>All</option>
												</select>
											</div>
										</div>
										<br/>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Year:</label>
											<div class="col-md-9">
												<input class="form-control" type="number" name="report_year" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Month:</label>
											<div class="radio-group col-md-9">
												<label for="month_agg_1"><input type="checkbox" name="month_agg_1" value="1" id="month_agg_1" checked />January</label>
												<label for="month_agg_2"><input type="checkbox" name="month_agg_2" value="2" id="month_agg_2" />February</label>
												<label for="month_agg_3"><input type="checkbox" name="month_agg_3" value="3" id="month_agg_3" />March</label>
												<label for="month_agg_4"><input type="checkbox" name="month_agg_4" value="4" id="month_agg_4" />April</label>
												<label for="month_agg_5"><input type="checkbox" name="month_agg_5" value="5" id="month_agg_5" />May</label>
												<label for="month_agg_6"><input type="checkbox" name="month_agg_6" value="6" id="month_agg_6" />June</label>
												<label for="month_agg_7"><input type="checkbox" name="month_agg_7" value="7" id="month_agg_7" />July</label>
												<label for="month_agg_8"><input type="checkbox" name="month_agg_8" value="8" id="month_agg_8" />August</label>
												<label for="month_agg_9"><input type="checkbox" name="month_agg_9" value="9" id="month_agg_9" />September</label>
												<label for="month_agg_10"><input type="checkbox" name="month_agg_10" value="10" id="month_agg_10" />October</label>
												<label for="month_agg_11"><input type="checkbox" name="month_agg_11" value="11" id="month_agg_11" />November</label>
												<label for="month_agg_12"><input type="checkbox" name="month_agg_12" value="12" id="month_agg_12" />December</label>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year"></label>
											<div class="col-md-9">
												<label for="month_agg"><input type="checkbox" name="month_agg" value="0" id="month_agg" style="width:30px;margin-top:10px" />Select all</label>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<div class="col-md-12">
												<input class="add-edit-btn" name="update_aggregate_reports" type="button" value="Show reports" style="border-radius:4px;font-size:14px" disabled>
											</div>
										</div>
									</div>
								</form>
								<div class="col-md-9 reports-aggregate">
									<div id="charts_aggregate"></div>
								</div>
							</div>
						</div>
					</div>
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion_statistics" href="#reports_orbit" class="collapsed" aria-expanded="false">Orbit Acquisition Reports</a>
							</h4>
						</div>						
						<div id="reports_orbit" class="panel-collapse collapse">
							<div class="panel-body">
								<form id="filters_orbit" role="form" method="post">
									<div class="col-md-3 filters">
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="site_select">Site:</label>
											<div class="col-md-9">
												<select class="form-control" name="site_select"><?php select_site();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="satellite_select">Satellite:</label>
											<div class="col-md-9">
												<select class="form-control" name="satellite_select"><?php select_satellite();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="orbit_select">Orbit:</label>
											<div class="col-md-9">
												<select size="6" class="form-control" name="orbit_select" style="height:auto">
													<option value="0" selected>All</option>
												</select>
											</div>
										</div>
										<br/>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Year:</label>
											<div class="col-md-9">
												<input class="form-control" type="number" name="report_year" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Month:</label>
											<div class="radio-group col-md-9">
												<label for="month_orb_1"><input type="radio" name="month_orb" value="1" id="month_orb_1" checked />January</label>
												<label for="month_orb_2"><input type="radio" name="month_orb" value="2" id="month_orb_2" />February</label>
												<label for="month_orb_3"><input type="radio" name="month_orb" value="3" id="month_orb_3" />March</label>
												<label for="month_orb_4"><input type="radio" name="month_orb" value="4" id="month_orb_4" />April</label>
												<label for="month_orb_5"><input type="radio" name="month_orb" value="5" id="month_orb_5" />May</label>
												<label for="month_orb_6"><input type="radio" name="month_orb" value="6" id="month_orb_6" />June</label>
												<label for="month_orb_7"><input type="radio" name="month_orb" value="7" id="month_orb_7" />July</label>
												<label for="month_orb_8"><input type="radio" name="month_orb" value="8" id="month_orb_8" />August</label>
												<label for="month_orb_9"><input type="radio" name="month_orb" value="9" id="month_orb_9" />September</label>
												<label for="month_orb_10"><input type="radio" name="month_orb" value="10" id="month_orb_10" />October</label>
												<label for="month_orb_11"><input type="radio" name="month_orb" value="11" id="month_orb_11" />November</label>
												<label for="month_orb_12"><input type="radio" name="month_orb" value="12" id="month_orb_12" />December</label>
												<br/><br/>
												<label for="custom_date" style="font-weight:700"><input type="radio" name="month_orb" value="0" id="custom_date" />Custom Interval</label>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-4" for="fromDate" style="text-align:right;padding-right:0">From:</label>
											<div class="col-md-8">
												<input class="form-control" type="date" name="fromDate" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-4" for="toDate" style="text-align:right;padding-right:0">To:</label>
											<div class="col-md-8">
												<input class="form-control" type="date" name="toDate" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<div class="col-md-12">
												<input class="add-edit-btn" name="update_orbit_report" type="button" value="Show report" style="float:right;border-radius:4px;font-size:14px" disabled>
											</div>
										</div>
									</div>
								</form>
								<div class="col-md-9 reports-orbit">
									<div id="chart_orbit"></div>
								</div>
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
</div>
<!-- includes moment.js for date manipulation -->
<script src="libraries/momentjs/moment.min.js" type="text/javascript"></script>
<!-- includes for datepicker and other controls -->
<link href="libraries/jquery-ui/jquery-ui.min.css" type="text/css" rel="stylesheet">
<script src="libraries/jquery-ui/jquery-ui.min.js" type="text/javascript"></script>
<!-- includes for radar chart -->
<link href="libraries/radar-chart/radar-chart.min.css" type="text/css" rel="stylesheet">
<script src="libraries/radar-chart/radar-chart.min.js" type="text/javascript"></script>

<script type="text/javascript">
window.chartsList = [];
window.radarChart = null;

function updateOrbitList(form) {
	var $orbit_select = $("select[name='orbit_select']", form);
	var option_orb = "<option value=\"0\" selected>All</option>";
	$orbit_select.html(option_orb);
	
	var satellite = $("select[name='satellite_select']", form).val();
	if (satellite != "" && satellite != "0") {
		var siteId = $("select[name='site_select']").val();
		var oData = {
			"report_type": "orbit",
			"getOrbitList": "yes",
			"satellite": satellite
		};
		if (siteId != "" && siteId != "0") {
			oData.siteId = siteId;
		}
		$.ajax({
			url: "getReports.php",
			data: oData,
			method: "GET",
			dataType: "json",
			success: function (response) {
				if (response.status == "SUCCEEDED") {
					$.each(response.data, function(index, value) {
						$orbit_select.append("<option value='" + value + "'>" + value + "</option>"); 
					});
				}
			},
			error: function (jqXHR, textStatus, errorThrown) {
			}
		});
	}
}
function getOrbitStatistics(satellite, siteId, orbit, fromDate, toDate) {
	var deferred = $.Deferred();
	
	var oData = {
		"report_type": "orbit",
		"satellite": satellite,
	};
	if (siteId != "" && siteId != "0") {
		oData.siteId = siteId;
	}
	if (orbit != "" && orbit != "0") {
		oData.orbit = orbit;
	}
	if (fromDate != "" && toDate != "") {
		oData.fromDate = fromDate;
		oData.toDate   = toDate;
	}
	$.ajax({
        async: false,
      	url: "getReports.php",
		data: oData,
		method: "GET",
		dataType: "json",
		success: function (response) {
			if (response.status == "SUCCEEDED") {
				deferred.resolve(response);
			} else if (response.status == "FAILED") {
				deferred.reject(null, "FAILED", response.message);
			}
        },
		error: function (jqXHR, textStatus, errorThrown) {
			deferred.reject(jqXHR, textStatus, "Error while retrieving statistics.");
		}
	});
	return deferred.promise();
}
function getAggregateStatistics(satellite, siteId, orbit, fromDate, toDate) {
	var deferred = $.Deferred();
	
	var oData = {
		"report_type": "aggregate",
		"satellite": satellite,
	};
	if (siteId != "" && siteId != "0") {
		oData.siteId = siteId;
	}
	if (orbit != "" && orbit != "0") {
		oData.orbit = orbit;
	}
	if (fromDate != "" && toDate != "") {
		oData.fromDate = fromDate;
		oData.toDate   = toDate;
	}
	$.ajax({
        async: false,
      	url: "getReports.php",
		data: oData,
		method: "GET",
		dataType: "json",
		success: function (response) {
			if (response.status == "SUCCEEDED") {
				deferred.resolve(response);
			} else if (response.status == "FAILED") {
				deferred.reject(null, "FAILED", response.message);
			}
        },
		error: function (jqXHR, textStatus, errorThrown) {
			deferred.reject(jqXHR, textStatus, "Error while retrieving statistics.");
		}
	});
	return deferred.promise();
}

function showRadar(satellite, siteId, orbit, fromDate, toDate) {
	var data = [];
	var size = $(".statistics .reports-orbit").width() - 50;
	
	$.when(getOrbitStatistics(satellite, siteId, orbit, fromDate, toDate))
	.done(function (response) {
		var header = "orbit";
		var body = (orbit == "0" ? "acquisitions_all" : "acquisitions_" + orbit);
		$.each(response.data, function(index, value) {
			header += "," + value.calendarDate.substring(5);
			body   += "," + value.acquisitions;
		});
		var report = header + "\n" + body;
		csv = report.split("\n").map(function (i) { return i.split(","); });
		var headers = [];
		csv.forEach(function (item, i) {
			if (i == 0) {
				headers = item;
			} else {
				newSeries = {};
				item.forEach(function(v, j) {
					if (j == 0) {
						newSeries.className = v;
						newSeries.axes = [];
					} else {
						newSeries.axes.push({ "axis": [headers[j]], "value": parseFloat(v) });
					}
				});
				data.push(newSeries);
			}
		});
		RadarChart.defaultConfig.w = size;
		RadarChart.defaultConfig.h = size;
		RadarChart.draw("#chart_orbit", data);
	})
	.fail(function (jqXHR, status, error) {
		alert(error);
	});
}
function showAggregate(id, satellite, siteId, orbit, fromDate, toDate) {
	$.when(getAggregateStatistics(satellite, siteId, orbit, fromDate, toDate))
	.done(function (response) {
		var pairs            = { "key": "Pairs"            , "color": "#4e4e4e", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var processed        = { "key": "Processed"        , "color": "#2eb32e", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var notYetProcessed  = { "key": "Not Yet Processed", "color": "#007bbb", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var falselyProcessed = { "key": "Falsely Processed", "color": "#ffc100", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var noIntersections  = { "key": "No Intersections" , "color": "#a5a5a5", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var errors           = { "key": "Errors"           , "color": "#bb0000", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var acquisitions     = { "key": "Acquisitions"     , "color": "#336cc1", "type": "line", "yAxis": 2, "values" : [ ] };
		// Add empty values for months with less than 31 days
		for (var i = response.data.length + 1; i < 32; i++) {
			var foo = {
				calendarDate: fromDate.substring(0, 8) + i,
				acquisitions: 0,
				downloadFailed: 0,
				processed: 0,
				notYetProcessed: 0,
				falselyProcessed: 0,
				errors: 0,
				pairs: 0,
				noIntersections: 0
			}
			response.data.push(foo);
		}
		var axisKeys = [];
		$.each(response.data, function(index, value) {
			pairs.values.push           ({ x: index, y: value.pairs });
			processed.values.push       ({ x: index, y: value.processed });
			notYetProcessed.values.push ({ x: index, y: value.notYetProcessed });
			notYetProcessed.values.push ({ x: index, y: value.processed });
			falselyProcessed.values.push({ x: index, y: value.falselyProcessed });
			noIntersections.values.push ({ x: index, y: value.noIntersections });
			errors.values.push          ({ x: index, y: value.errors });
			acquisitions.values.push    ({ x: index, y: value.acquisitions });
			axisKeys.push(value.calendarDate.substring(8));
		});
		nv.addGraph({
			generate: function() {
				var chart = nv.models.multiChart();
				chart.margin({ top: 10, right: 40, bottom: 20, left: 30 });
				chart.legend.margin({top: -60, right: 0, left: 0, bottom: 0});
				chart.xAxis
					.ticks(31)
					.tickFormat(function (d) {
						return (typeof axisKeys[d] !== "undefined") ? parseInt(axisKeys[d]) : "";
					});
				chart.yAxis1
					.tickFormat(function (d){ return (d == parseInt(d) ? d3.format('1.0f')(d) : ""); })
				chart.yAxis2
					.tickFormat(function (d){ return (d == parseInt(d) ? d3.format('1.0f')(d) : ""); })
					.axisLabel("Acquisitions")
					.axisLabelDistance(-40);
				var seriesData = [];
				seriesData.push(acquisitions);
				seriesData.push(pairs);
				seriesData.push(processed);
				seriesData.push(notYetProcessed);
				seriesData.push(falselyProcessed);
				seriesData.push(noIntersections);
				seriesData.push(errors);
				
				// Get and set max values for each axis
				var minY1 = 0;
				var minY2 = 0;
				var maxY1 = 0;
				var maxY2 = 0;
				for (var i = 0; i < seriesData.length; i++) {
					var _axis = seriesData[i].yAxis;
					var _values = seriesData[i].values;

					// Walk values and set largest to variables
					for (var j = 0; j < _values.length; j++) {
						// For maxY1
						if (_axis == 1) {
							if(_values[j].y > maxY1) {
								maxY1 = _values[j].y;
							}
						}
						// For maxY2
						if (_axis == 2) {
							if (_values[j].y > maxY2) {
								maxY2 = _values[j].y;
							}
						}
					}
				}
				chart.yDomain1([minY1, maxY1]);
				chart.yDomain2([minY2, maxY2]);
				
				var  satName = $("select[name='satellite_select']>option[value='" + satellite + "']", "form#filters_aggregate").text()
				var title = satName + " Preprocessing for <span>" + moment(id, "MM").format("MMMM") + " " + moment(fromDate, "YYYY-MM-DD").format("YYYY") + "</span>";
				
				$("#charts_aggregate").append("<div id='chart_aggregate_" + id + "' class='chart-container'><div class='chart-title'>" + title + "</div><svg> </svg></div>");
				d3.select("#chart_aggregate_" + id + "> svg")
					.datum(seriesData)
					.call(chart);
				
				nv.utils.windowResize(chart.update);
				
				return chart;
			},
			callback: function (chart) {
				window.chartsList.push(chart);
				// update previous charts limits
				var minY1 = 0;
				var maxY1 = 0;
				var minY2 = 0;
				var maxY2 = 0;
				$.each(chartsList, function () {
					minY1 = Math.min(minY1, this._options["yDomain1"][0]);
					maxY1 = Math.max(maxY1, this._options["yDomain1"][1]);
					minY2 = Math.min(minY2, this._options["yDomain2"][0]);
					maxY2 = Math.max(maxY2, this._options["yDomain2"][1]);
				});
				$.each(chartsList, function () {
					this.yDomain1([minY1, maxY1]);
					this.yDomain2([minY2, maxY2]);
					this.update();
				});
			}
		});
	})
	.fail(function (jqXHR, status, error) {
		alert(error);
	});
}
function showAllAggregateReports() {
	// Make sure year is set
	if (isNaN($("input[name='report_year']", "form#filters_aggregate").val())) {
		$("input[name='report_year']", "form#filters_aggregate").val(new Date().getFullYear());
	}
	// Get filters values
	var sat    = $("select[name='satellite_select']", "form#filters_aggregate").val();
	var siteId = $("select[name='site_select']"     , "form#filters_aggregate").val();
	var orbit  = $("select[name='orbit_select']"    , "form#filters_aggregate").val();
	var year   = $("input[name='report_year']"      , "form#filters_aggregate").val();
	if (sat != "0" && $(".radio-group input[type='checkbox']:checked", "form#filters_aggregate").length > 0) {
		// Remove previous charts tooltips
		$.each(window.chartsList, function () {
			if (typeof this.tooltip() !== "undefined" && $(this.tooltip.node()).length > 0) {
				$(this.tooltip.node()).remove();
			} 
		});
		// Remove previous charts
		window.chartsList = [];
		$(".reports-aggregate").css({ "background-image": "none" });
		$("#charts_aggregate svg").remove();
		$("#charts_aggregate>*").remove();
		$.each($(".radio-group input[type='checkbox']:checked", "form#filters_aggregate"), function () {
			var fromDate = moment(year + "-" + this.value + "-01", "YYYY-MM-DD").format("YYYY-MM-DD");
			var toDate   = moment(fromDate, "YYYY-MM-DD").add(1, "month").add(-1, "day").format("YYYY-MM-DD");
			showAggregate(this.value, sat, siteId, orbit, fromDate, toDate);
		});
	}
}
function setFromToDate(year, month) {
	if (typeof year === "undefined") {
		var selYear = $("input[name='report_year']", "form#filters_orbit").val();
		if (isNaN(selYear)) {
			selYear = new Date().getFullYear();
			$("input[name='report_year']", "form#filters_orbit").val(selYear);
		}
		setFromToDate(selYear);
	} else if (typeof month === "undefined") {
		var $selMonth = $("input[name='month_orb']:checked", "form#filters_orbit")
		if ($selMonth.length > 0 && $selMonth.get(0).id !== "custom_date") {
			setFromToDate(year, $selMonth.val());
		} else {
			var fromDate = $("input[name='fromDate']", "form#filters_orbit").get(0).valueAsDate;
			var toDate   = $("input[name='toDate']"  , "form#filters_orbit").get(0).valueAsDate;
			if (fromDate) {
				fromDate = moment(year + "-" + (fromDate.getMonth() + 1) + "-" + fromDate.getDate(), "YYYY-MM-DD").format("YYYY-MM-DD");
				$("input[name='fromDate']", "form#filters_orbit").val(fromDate);
			}
			if (toDate) {
				toDate = moment(year + "-" + (toDate.getMonth() + 1) + "-" + toDate.getDate(), "YYYY-MM-DD").format("YYYY-MM-DD");
				$("input[name='toDate']", "form#filters_orbit").val(toDate);
			}
		}
	} else {
		var fromDate = moment(year + "-" + month + "-01", "YYYY-MM-DD").format("YYYY-MM-DD");
		var toDate   = moment(fromDate, "YYYY-MM-DD").add(1, "month").add(-1, "day").format("YYYY-MM-DD");
		$("input[name='fromDate']", "form#filters_orbit").val(fromDate);
		$("input[name='toDate']"  , "form#filters_orbit").val(toDate);
	}
}
/* =========== START =========== */
$(document).ready(function() {
	//$("select[name=satellite_select]").val("S1");
	//$("input[name=update_orbit_report]").prop("disabled", false);
	//$("input[name=update_aggregate_reports]").prop("disabled", false);
	
	// Set filter default year
	var crtYear = (new Date().getFullYear() - 1 ) + "";
	$("input[name='report_year']").val(crtYear);
	setFromToDate();
	
	// Aggregate filters changes
	$("form#filters_aggregate")
	.on("change", "#month_agg", function (e) {
		$(".radio-group input[type='checkbox']", "form#filters_aggregate").prop("checked", $(this).is(":checked"));
	})
	.on("change", "select[name='satellite_select']", function (e) {
		if (this.value == "0") {
			$("input[name='update_aggregate_reports']", e.delegateTarget).prop("disabled", true);
		} else {
			$("input[name='update_aggregate_reports']", e.delegateTarget).prop("disabled", false);
		}
		updateOrbitList(e.delegateTarget);
	})
	.on("change", "select[name='site_select']", function (e) {
		updateOrbitList(e.delegateTarget);
	});
	
	// Display selected aggregate reports
	$("input[name='update_aggregate_reports']").on("click", function (e) {
		showAllAggregateReports();
	});
	
	// Orbit filters changes
	$("form#filters_orbit")
	.on("change", "input[name='report_year']", function (e) {
		setFromToDate(this.value);
	})
	.on("change", "input[name='month_orb']", function (e) {
		if (this.id != "custom_date") {
			setFromToDate();
		}
	})
	.on("change", "input[type='date']", function (e) {
		$("#custom_date").prop("checked", true);
	})
	.on("change", "select[name='satellite_select']", function (e) {
		if (this.value == "0") {
			$("input[name='update_orbit_report']", e.delegateTarget).prop("disabled", true);
		} else {
			$("input[name='update_orbit_report']", e.delegateTarget).prop("disabled", false);
		}
		updateOrbitList(e.delegateTarget);
	})
	.on("change", "select[name='site_select']", function (e) {
		updateOrbitList(e.delegateTarget);
	});
	// Display orbit report
	$("input[name='update_orbit_report']").on("click", function (e) {
		var d1 = Date.parse($("input[name=fromDate").get(0).valueAsDate);
		var d2 = Date.parse($("input[name=toDate").get(0).valueAsDate);
		if (isNaN(d1) || isNaN(d2)) {
			alert("Please select valid dates for your report!");
		} else {
			if (d1 > d2) {
				var dt = $("input[name='toDate']").val();
				$("input[name='toDate']").val($("input[name='fromDate']").val());
				$("input[name='fromDate']").val(dt);
			}
			var sat      = $("select[name='satellite_select']", "form#filters_orbit").val();
			var siteId   = $("select[name='site_select']"     , "form#filters_orbit").val();
			var orbit    = $("select[name='orbit_select']"    , "form#filters_orbit").val();
			var fromDate = $("input[name='fromDate']"         , "form#filters_orbit").val();
			var toDate   = $("input[name='toDate']"           , "form#filters_orbit").val();
			if (sat != "0") {
				$(".reports-orbit").css({ "background-image": "none" });
				
				$("#chart_orbit svg").remove();
				$("#chart_orbit>*").remove();
				
				showRadar(sat, siteId, orbit, fromDate, toDate);
			}
		}
	});
});

//# sourceURL=statistics.js
</script>

<?php include 'ms_foot.php'; ?>
