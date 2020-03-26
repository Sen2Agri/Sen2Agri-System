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
			<style>
				.chart-title{color:#81a53e;font-weight:700;text-align:center}
				.chart-title>span{color:#336cc1}
				
				/* ==== General chart settings ==== */
				#charts_aggregate path.nv-line{stroke-width:3px}
				#charts_aggregate line.nv-guideline{stroke:#346cc1;stroke-dasharray:5}
				
				/* ==== Aggregate Chart adjustments ==== */
				#charts_aggregate svg{margin-bottom:20px;height:300px;width:100%}
				#charts_aggregate .nv-axislabel{font:15px Arial,Helvetica,sans-serif;font-weight:700;fill:#336cc1}
				/* ---- Legend adjustments ---- */
				.chart-container:first-child{margin-top:105px}
				#charts_aggregate .chart-container:first-child svg{overflow:visible}
				#charts_aggregate .chart-container:first-child::after{
					content:"Click / Double-click \A legend labels \A to customize your selection";
					position:absolute;white-space:pre;background-color:#edf2e4;top:15px;left:50px;font-size:12px;
					color: green;text-align:left;opacity:0;padding:10px;border-radius:10px;transition:opacity 2s;
				}
				#charts_aggregate.show-legend-info .chart-container:first-child::after{opacity:1}
				/* ---- Adjust series (bars and line) ---- */
				#charts_aggregate svg g.nv-x text{transform:translate(10px,0)}														/* ---- Shift line and oX labels to align to the center of the tick  ---- */
				#charts_aggregate svg g.nv-x g.nv-axisMax-x{display:none}															/* ---- Hide last oX label ---- */
				#charts_aggregate svg g.nv-multibar{transform:scale(1.033,1)}														/* ---- Scale bars to match 31 tickes instead of 30  ---- */
				#charts_aggregate svg g.nv-multibar g.nv-group rect[style="fill: transparent; stroke: transparent;"]{display:none}	/* ---- Hide all bars that are marked as invalid (transparent color) ---- */
				
				/* ==== Orbit Chart adjustments ==== */
				#chart_orbit{position:relative}
				#chart_orbit svg{overflow:visible}
				#chart_orbit text{user-select:none;-ms-user-select:none;pointer-events:none;fill:#aaaaaa;transition:opacity 1s}
				#chart_orbit text.axisLabel{font-size:16px !important;font-weight:700;fill:#7eaf50}
				#chart_orbit text.tooltip{font-size: 18px;font-weight:700;fill:#336cc1}
				#chart_orbit svg.hover .axis line.hover{stroke-dasharray:5 3;stroke-width:1px !important;stroke:#7eaf50 !important;display:block !important}
				#chart_orbit svg.hover .axis text{opacity:0}
				#chart_orbit text.tooltip.hover{opacity:1!important;text-shadow:1px 1px 1px #d6deea}
				
				#radar_tooltip{display:none}
				
				/* ==== Customize Chart tooltip ==== */
				.nvtooltip.xy-tooltip{position:fixed !important}
				.nvtooltip table{margin:0}
				.nvtooltip table th{padding:3px 6px 3px 5px;background-color:#edf2e4;text-align:center}
				.nvtooltip table td{padding:3px 6px 3px 5px;font-size:12px;line-height:10px;}
				.nvtooltip table td div{width:10px;height:10px}
				.nvtooltip table tr.line td{font-weight:700}
				
				.statistics *{box-sizing:border-box}
				.statistics .panel-body{border:0}
				.statistics .form-group-sm{margin-bottom:5px}
				.statistics .form-group-sm .form-control{height:30px;line-height:20px}
				.statistics .form-group-sm .control-label{font-size:14px;line-height:25px}
				.statistics .form-group-sm .radio-group{font-size:14px}
				.statistics .form-group-sm .radio-group input{width:30px}
				.statistics .form-group-sm .radio-group label{width:100%;font-weight:400;margin:0}
				.statistics .filters{background-color:#edf2e4;padding:10px;border:1px solid #dddddd;border-radius:5px}
				.statistics .reports-aggregate{background:transparent url(images/chart.png) no-repeat 50% 50%;min-height:600px;padding:10px;text-align:center;overflow:hidden}
				.statistics .reports-orbit    {background:transparent url(images/radar.png) no-repeat 50% 50%;min-height:600px}
				.statistics .add-edit-btn{width:100%;font-size:14px;text-align:center;margin-top:10px}
				.statistics .add-edit-btn span{color:white;display:none}
				.statistics .add-edit-btn.pending-btn span{display:inline-block}
				.statistics h4.panel-title a,.statistics h4.panel-title span{width:100%}
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
											<label class="control-label col-md-3" for="site_select">Site</label>
											<div class="col-md-9">
												<select class="form-control" name="site_select"><?php select_site();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="satellite_select">Satellite</label>
											<div class="col-md-9">
												<select class="form-control" name="satellite_select"><?php select_satellite();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="orbit_select">Orbit</label>
											<div class="col-md-9">
												<select size="6" class="form-control" name="orbit_select" style="height:auto">
													<option value="0" selected>All</option>
												</select>
											</div>
										</div>
										<br/>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Year</label>
											<div class="col-md-9">
												<input class="form-control" type="number" name="report_year" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Month</label>
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
											<div class="col-md-12 add-edit-btn-parent loading">
												<button type="button" class="add-edit-btn" name="update_aggregate_reports" disabled>Show reports&nbsp;&nbsp;<span class="glyphicon glyphicon-repeat glyphicon-spin"></span></button>
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
								<div class="col-md-3 filters">
									<form id="filters_orbit" role="form" method="post">
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="site_select">Site</label>
											<div class="col-md-9">
												<select class="form-control" name="site_select"><?php select_site();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="satellite_select">Satellite</label>
											<div class="col-md-9">
												<select class="form-control" name="satellite_select"><?php select_satellite();?></select>
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="orbit_select">Orbit</label>
											<div class="col-md-9">
												<select size="6" class="form-control" name="orbit_select" style="height:auto">
													<option value="0" selected>All</option>
												</select>
											</div>
										</div>
										<br/>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Year</label>
											<div class="col-md-9">
												<input class="form-control" type="number" name="report_year" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-3" for="report_year">Month</label>
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
											<label class="control-label col-md-4" for="fromDate" style="text-align:right;padding-right:0">From</label>
											<div class="col-md-8">
												<input class="form-control" type="date" name="fromDate" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<label class="control-label col-md-4" for="toDate" style="text-align:right;padding-right:0">To</label>
											<div class="col-md-8">
												<input class="form-control" type="date" name="toDate" />
											</div>
										</div>
										<div class="row form-group form-group-sm">
											<div class="col-md-12">
												<button type="button" class="add-edit-btn" name="update_orbit_report" disabled>Show report&nbsp;&nbsp;<span class="glyphicon glyphicon-repeat glyphicon-spin"></span></button>
											</div>
										</div>
									</form>
								</div>
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
<script src="libraries/radar-chart/RadarChart.js" type="text/javascript"></script>
<script type="text/javascript">
window.chartsList = [];
window.radarChart = null;
/* =========== GET DATA =========== */
function getOrbitList(forms) {
	var $forms = (typeof forms === "undefined" ? $("form", ".statistics") : $(forms));
	$.each($forms, function (index, form) {
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
				cache: false,
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
	});
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
		cache: false,
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
		cache: false,
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
/* =========== SHOW REPORTS =========== */
function showOrbit(satellite, siteId, orbit, fromDate, toDate) {
	var data = [];
	var size = $(".statistics .reports-orbit").width() - 100;
	
	$.when(getOrbitStatistics(satellite, siteId, orbit, fromDate, toDate))
	.done(function (response) {
		if (typeof response.data === "undefined" || typeof response.data.columnLabels === "undefined" || typeof response.data.series === "undefined") {
			setButtonStatus($("button[name='update_orbit_report']"), true, false);
			return;
		}
		
		var oxVariableName = "calendarDate";
		var seriesLineName = "acquisitions";
		
		var newSeries = [];
		var maxValue = 1;
		$.each(response.data.series, function(index, value) {
			newSeries.push({ "axis": value[oxVariableName].substring(5), "value": value[seriesLineName], key: response.data.columnLabels[seriesLineName] , "date": value[oxVariableName] });
			maxValue = Math.max(maxValue, value[seriesLineName]);
		});
		data.push(newSeries);
		$("#chart_orbit").addClass("ticks-" + newSeries.length);
		
		var color = d3.scale.ordinal()
			.range([ "#7EAF50", "#EDC951", "#CC333F" ]);
		var margin = { top: 50, right: 50, bottom: 50, left: 50 },
			width = size - margin.left - margin.right,
			height = width;
		var radarChartOptions = {
			w: size,
			h: size,
			margin: margin,
			maxValue: maxValue,
			levels: maxValue,
			dotRadius: 4,
			color: color,
			labelFactor: 1.08,
			maxLabels: 60
		};
		RadarChart("#chart_orbit", data, radarChartOptions);
		
		setButtonStatus($("button[name='update_orbit_report']"), true, false);
		
		d3.selectAll(".radarInvisibleCircle")
		.on("mouseover", function (d, i) {
			var $svg = $("#chart_orbit svg");
			var crtYear = $("input[name=report_year]", "#filters_orbit").val();
			newX =  parseFloat(d3.select(this).attr('cx')) + 10 + $svg.offset().left - $svg.width()/2;
			newY =  parseFloat(d3.select(this).attr('cy'))  + $svg.offset().top;
			
			$svg.attr("class", $svg.attr("class") + " hover");
			
			var $line = $(".axisWrapper .axis .line-" + d.date);
			$line.attr("class", $line.attr("class") + " hover");
			
			var $legend = $(".axisWrapper .axis .legend-" + d.date);
			$legend.attr("class", $legend.attr("class") + " hover");
			
			var $tooltip = $("text.tooltip", "#chart_orbit");
			$tooltip.attr("class", $tooltip.attr("class") + " hover");
			$tooltip.attr("x", $legend.attr("x") - 50);
			$tooltip.attr("y", $legend.attr("y") + 5);
			$tooltip.text(d.date + ": "+ d.value);
			
			//var o = { index: 0, series: [{ key: d.key, value: d.value, color: "#7eaf50" }], axisKeys: [{ date: d.date }] };		
			//$tooltip = $("#radar_tooltip");
			//$tooltip.html("").append(tooltipContentHtml(o));
			//$tooltip.css({ "display": "block", "left": newX + "px", "top": newY + "px" });
		})
		.on("mouseout", function (d, i){
			var $svg = $("#chart_orbit svg");
			$svg.attr("class", $svg.attr("class").replace(" hover", ""));
			
			var $line = $(".axisWrapper .axis .line-" + d.date);
			$line.attr("class", $line.attr("class").replace(" hover", ""));
			
			var $legend = $(".axisWrapper .axis .legend-" + d.date);
			$legend.attr("class", $legend.attr("class").replace(" hover", ""));
			
			var $tooltip = $("text.tooltip", "#chart_orbit");
			$tooltip.attr("class", $tooltip.attr("class").replace(" hover", ""));
			
			//$tooltip = $("#radar_tooltip");
			//$tooltip.css({ "display": "none" });
		});
		
	})
	.fail(function (jqXHR, status, error) {
		alert(error);
		setButtonStatus($("button[name='update_orbit_report']"), true, false);
	});
}
function showOrbitReports() {
	var d1 = $("input[name=fromDate").get(0);
	var d2 = $("input[name=toDate").get(0);
	d1 = Date.parse(typeof d1.valueAsDate === "undefined" ? d1.value : d1.valueAsDate);
	d2 = Date.parse(typeof d2.valueAsDate === "undefined" ? d2.value : d2.valueAsDate);
	if (isNaN(d1) || isNaN(d2)) {
		alert("Please select valid dates for your report!");
		setButtonStatus($("button[name='update_orbit_report']"), true, false);
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
			$("#chart_orbit").removeClass();
			var satName = $("select[name='satellite_select']>option[value='" + sat + "']", "form#filters_orbit").text();
			var title = satName + " Orbit <span>" + (orbit == "0" ? "" : orbit) + "</span> Acquisitions from <span>" + fromDate + "</span> to <span>" + toDate + "</span>";
			$("#chart_orbit").append("<div class='chart-title' class='nvtooltip'>" + title + "</div>");
			$("#chart_orbit").append("<div id='radar_tooltip' class='nvtooltip'></div>");
			showOrbit(sat, siteId, orbit, fromDate, toDate);
		}
	}
}
function showAggregate(id, satellite, siteId, orbit, fromDate, toDate) {
	$.when(getAggregateStatistics(satellite, siteId, orbit, fromDate, toDate))
	.done(function (response) {
		if (typeof response.data === "undefined" || typeof response.data.columnLabels === "undefined" || typeof response.data.series === "undefined") {
			setButtonStatus($("button[name='update_aggregate_reports']"), true, false);
			return;
		}
		
		// define colors
		var colors = [];
		var oxVariableName = "calendarDate";
		var seriesLineName = "acquisitions";
		colors[seriesLineName    ] = "#336cc1";
		colors["downloadFailed"  ] = "#336cc1";
		colors["processed"       ] = "#2eb32e";
		colors["notYetProcessed" ] = "#007bbb";
		colors["falselyProcessed"] = "#ffc100";
		colors["errors"          ] = "#bb0000";
		colors["pairs"           ] = "#4e4e4e";
		colors["noIntersections" ] = "#a5a5a5";
		colors["clouds"          ] = "#cccccc";
		colors["foo"             ] = "#555555";
		// define series structure
		var series = {};
		var foo = {};
		$.each(response.data.columnLabels, function (key, value) {
			if (key != oxVariableName) {
				series[key] = {
					"key"   : value,
					"color" : (typeof colors[key] !== "undefined" ?  colors[key] : colors["foo"]),
					"type"  : (key == seriesLineName ? "line" : "bar"),
					"yAxis" : (key == seriesLineName ? 2 : 1),
					"values": []
				};
				foo[key] = 0;
			}
		});
		for (var i = response.data.series.length + 1; i < 32; i++) {
			var fooX = $.extend({}, foo);
			fooX[oxVariableName] = fromDate.substring(0, 8) + i;
			response.data.series.push(fooX);
		}
		// extract series values and define xAxis labels
		var axisKeys = [];
		$.each(response.data.series, function(index, rec) {
			var crtDate = rec[oxVariableName];
			var validDate = (moment(crtDate, "YYYY-MM-DD").format("YYYY-MM-DD") == crtDate);
			$.each(rec, function (key, value) {
				if (key != oxVariableName) {
					series[key].values.push(validDate ? { x: index, y: value } : { x: index, y: value, color: "transparent" });
					// add an invisible extra tick in order to correctly align chart bars and lines
					if (index == 30) { series[key].values.push({ x: index + 1, y: 0, color: "transparent" }); }
				} else {
					axisKeys.push(validDate ? { date: value, label: parseInt(value.substring(8)) } : { date: "-", label: "-" });
					// add extra label
					if (index == 30) { axisKeys.push({ date: "-", label: "-" }); }
				}
			});
		});
		// generate chart
		nv.addGraph({
			generate: function() {
				var chart = nv.models.multiChart();
				chart.margin({ top: 10, right: 40, bottom: 20, left: 30 });
				chart.legendRightAxisHint("");
				chart.legend.margin({top: -60, right: 0, left: 0, bottom: 0});
				chart.xAxis
					.ticks(axisKeys.length)
					.tickFormat(function (d) {
						return axisKeys[d].label;
					});
				chart.yAxis1
					.tickFormat(function (d){ return (d == parseInt(d) ? d3.format('1.0f')(d) : ""); })
					.axisLabelDistance(-40);
				chart.yAxis2
					.tickFormat(function (d){ return (d == parseInt(d) ? d3.format('1.0f')(d) : ""); })
					.axisLabel(series[seriesLineName].key)
					.axisLabelDistance(-40);
				chart.useInteractiveGuideline(true);
				chart.interactiveLayer.tooltip.contentGenerator(function (d, e) {
					if (axisKeys[d.index].label == "-") {
						$(e).addClass("hidden");
					} else {
						$(e).removeClass("hidden");
					}
					d.axisKeys = axisKeys;
					return tooltipContentHtml(d);
				});
				chart.interactiveLayer.tooltip.enabled(false);
				
				var seriesData = $.map(series, function (value, key) { return value; });
				var minY1 = 0;
				var maxY1 = 1;
				var minY2 = 0;
				var maxY2 = 1;
				for (var i = 0; i < seriesData.length; i ++) {
					var _axis = seriesData[i].yAxis;
					var _values = seriesData[i].values;
					for (var j = 0; j < _values.length; j ++) {
						if (_axis == 1) { maxY1 = (_values[j].y > maxY1) ? _values[j].y : maxY1; }
						if (_axis == 2) { maxY2 = (_values[j].y > maxY2) ? _values[j].y : maxY2; }
					}
				}
				chart.yDomain1([minY1, maxY1]);
				chart.yDomain2([minY2, maxY2]);
				
				var satName = $("select[name='satellite_select']>option[value='" + satellite + "']", "form#filters_aggregate").text();
				var title = satName + " Preprocessing for <span>" + moment(id, "MM").format("MMMM") + " " + moment(fromDate, "YYYY-MM-DD").format("YYYY") + "</span><span>" + (orbit == "0" ? "" : " [Orbit " + orbit + "]") + "</span>";
				
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
				var maxY1 = 1;
				var minY2 = 0;
				var maxY2 = 1;
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
				var count = $(".radio-group input[type='checkbox']:checked", "form#filters_aggregate").length;
				if (chartsList.length == count) {
					setButtonStatus($("button[name='update_aggregate_reports']"), true, false);
					showLegendInfo();
					$.each(chartsList, function () {
						this.interactiveLayer.tooltip.enabled(true);
					});
				}
			}
		});
	})
	.fail(function (jqXHR, status, error) {
		alert(error);
		setButtonStatus($("button[name='update_aggregate_reports']"), true, false);
	});
}
function showAllAggregateReports() {
	// Make sure year is set
	if (isNaN($("input[name='report_year']", "form#filters_aggregate").val())) {
		$("input[name='report_year']", "form#filters_aggregate").val(new Date().getFullYear());
		setButtonStatus($("button[name='update_aggregate_reports']"), true, false);
	}
	// Get filters values
	var sat    = $("select[name='satellite_select']", "form#filters_aggregate").val();
	var siteId = $("select[name='site_select']"     , "form#filters_aggregate").val();
	var orbit  = $("select[name='orbit_select']"    , "form#filters_aggregate").val();
	var year   = $("input[name='report_year']"      , "form#filters_aggregate").val();
	if (sat != "0" && $(".radio-group input[type='checkbox']:checked", "form#filters_aggregate").length > 0) {
		// Remove previous chart tooltips
		$(".nvtooltip.xy-tooltip").remove();
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
	} else {
		setButtonStatus($("button[name='update_aggregate_reports']"), true, false);
	}
}
/* =========== GENERAL FUNCTIONS =========== */
function showLegendInfo() {
	// Prevent this functionality in IE
	if (typeof(Event) === 'function') {
		if (!$("#charts_aggregate").hasClass("show-legend-info")) {
			$("#charts_aggregate").addClass("show-legend-info");
			setTimeout(function () { $("#charts_aggregate").removeClass("show-legend-info"); }, 3000);
		}
	}
}
function tooltipContentHtml(d) {
	var $table = $("<table></table>");
	var $thead = $("<thead><tr><th colspan='3'><strong class='x-value'>" + d.axisKeys[d.index].date + "</strong></th></tr></thead>");
	var $tbody = $("<tbody></tbody>");
	$.each(d.series, function (index, s) {
		var tr = "";
		if (typeof s.yAxis === "function" && typeof s.yAxis.axisLabel === "function") {
			tr = "<tr class='" + (s.key === s.yAxis.axisLabel() ? "line" : (s.value == 0 ? "hidden" : "")) + "'>" +
					"<td><div style='background-color:" + s.color + "'></div></td>" +
					"<td" + (s.key === s.yAxis.axisLabel() ? " style='color:" + s.color + "'" : "") + ">" + s.key + "</td>" +
					"<td class='value'>" + s.value + "</td>" +
				"</tr>";
		} else {
			tr = "<tr class='line'>" +
					"<td><div style='background-color:" + s.color + "'></div></td>" +
					"<td style='color:" + s.color + "'>" + s.key + "</td>" +
					"<td class='value'>" + s.value + "</td>" +
				"</tr>";
		}
		$tbody.append(tr);
	});
	$table.append($thead).append($tbody);
	return $table[0].outerHTML;
}
function setButtonStatus($btn, enabled, pending) {
	$btn.prop("disabled", !enabled);
	if (pending) {
		$btn.addClass("pending-btn");
	} else {
		$btn.removeClass("pending-btn");
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
		var siteId = $("select[name='site_select']", e.delegateTarget).val();
		setButtonStatus($("button[name='update_aggregate_reports']"), (this.value != "0") && (siteId != "0"));
		getOrbitList(e.delegateTarget);
	})
	.on("change", "select[name='site_select']", function (e) {
		var sattelite = $("select[name='satellite_select']", e.delegateTarget).val();
		setButtonStatus($("button[name='update_aggregate_reports']"), (this.value != "0") && (sattelite != "0"));
		getOrbitList(e.delegateTarget);
	});
	// Display selected aggregate reports
	$("button[name='update_aggregate_reports']").on("click", function (e) {
		setButtonStatus($(this), false, true);
		setTimeout(function () { showAllAggregateReports(); }, 200);
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
		var siteId = $("select[name='site_select']", e.delegateTarget).val();
		setButtonStatus($("button[name='update_orbit_report']"), (this.value != "0") && (siteId != "0"));
		getOrbitList(e.delegateTarget);
	})
	.on("change", "select[name='site_select']", function (e) {
		var sattelite = $("select[name='satellite_select']", e.delegateTarget).val();
		setButtonStatus($("button[name='update_orbit_report']"), (this.value != "0") && (sattelite != "0"));
		getOrbitList(e.delegateTarget);
	});
	// Display orbit report
	$("button[name='update_orbit_report']").on("click", function (e) {
		setButtonStatus($(this), false, true);
		setTimeout(function () { showOrbitReports(); }, 200);
	});
	
	// Chart legend events
	$("#charts_aggregate")
	.on("click dblclick", ".nv-series", function(e) {
		var label = $(this).index() + 1;
		$("#charts_aggregate>div:not(:first-child) g.legendWrap g.nv-series:nth-child(" + label + ")")
		d3.selectAll("#charts_aggregate>div:not(:first-child) g.legendWrap g.nv-series:nth-child(" + label + ")")
        .each(function(d) {
			// Prevent this functionality in IE
			if (typeof(Event) === 'function') {
				this.dispatchEvent(new Event(e.type));
			}
        });
	})
	.on("mouseenter", "g.legendWrap", function(e) {
		showLegendInfo();
	});
	// 
	//$("select[name='site_select']").val("53");
	//$("select[name='satellite_select']").val("S1").trigger("change");
});

//# sourceURL=statistics.js
</script>

<?php include 'ms_foot.php'; ?>
