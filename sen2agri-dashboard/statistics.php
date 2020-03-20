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
				.statistics .row *{box-sizing:border-box}
				.statistics .form-group-sm{margin-bottom:5px}
				.statistics .form-group-sm .form-control{height:30px}
				.statistics .form-group-sm .control-label{font-size:14px;line-height:26px}
				.statistics .form-group-sm .radio-group{font-size:14px}
				.statistics .form-group-sm .radio-group input{width:30px}
				.statistics .form-group-sm .radio-group label{width:100%;font-weight:400;margin:0}
				.statistics .filters{background-color:#edf2e4;padding:10px;border:1px solid #dddddd;border-radius:5px}
				.statistics .reports-orbit,.statistics .reports-aggregate{border:1px solid #edf2e4;padding:10px;text-align:center}
				.statistics .reports-aggregate{border-width:1px 1px 1px 0;background:transparent url(images/chart.png) no-repeat 50% 50%;min-height:250px}
				.statistics .reports-orbit{border-width:1px 1px 1px 0;background:transparent url(images/radar.png) no-repeat 50% 50%}
				.statistics input.add-edit-btn{width:100%;border-radius:4px;font-size:14px;text-align:center;margin-top:10px}
				.nvd3 g.nv-groups path.nv-line{stroke-width:3px}
			</style>
			<div class="panel panel-default config for-table statistics">
				<div class="panel-heading"><h4 class="panel-title"><span>Orbit reports</span></h4></div>
				<div class="panel-body">
					<!-- Select site and satellite -->
					<div class="row" style="padding:0 15px">
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
									<label for="month_no_1"><input type="radio" name="month_no" value="1" id="month_no_1" data-last="31" checked />January</label>
									<label for="month_no_2"><input type="radio" name="month_no" value="2" id="month_no_2" data-last="28" />February</label>
									<label for="month_no_3"><input type="radio" name="month_no" value="3" id="month_no_3" data-last="31" />March</label>
									<label for="month_no_4"><input type="radio" name="month_no" value="4" id="month_no_4" data-last="30" />April</label>
									<label for="month_no_5"><input type="radio" name="month_no" value="5" id="month_no_5" data-last="31" />May</label>
									<label for="month_no_6"><input type="radio" name="month_no" value="6" id="month_no_6" data-last="30" />June</label>
									<label for="month_no_7"><input type="radio" name="month_no" value="7" id="month_no_7" data-last="31" />July</label>
									<label for="month_no_8"><input type="radio" name="month_no" value="8" id="month_no_8" data-last="31" />August</label>
									<label for="month_no_9"><input type="radio" name="month_no" value="9" id="month_no_9" data-last="30" />September</label>
									<label for="month_no_10"><input type="radio" name="month_no" value="10" id="month_no_10" data-last="31" />October</label>
									<label for="month_no_11"><input type="radio" name="month_no" value="11" id="month_no_11" data-last="30" />November</label>
									<label for="month_no_12"><input type="radio" name="month_no" value="12" id="month_no_12" data-last="31" />December</label>
									<br/><br/>
									<label for="custom_date" style="font-weight:700"><input type="radio" name="month_no" value="0" id="custom_date" />Custom Interval</label>
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
									<input class="add-edit-btn" name="update_orbit_report" type="button" value="Update report" style="float:right;border-radius:4px;font-size:14px" disabled>
								</div>
							</div>
						</div>
						<div class="col-md-9 reports-orbit">
							<div id="chart_orbit"></div>
						</div>
					</div>
					<div class="row" style="padding:0 15px">
						<div class="col-md-12 reports-aggregate">
							<div id="chart_aggregate"></div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
</div>
<script type="text/javascript" src="libraries/radar-chart/radar-chart.min.js"></script>
<link rel="stylesheet" href="libraries/radar-chart/radar-chart.min.css">

<script type="text/javascript">
function updateOrbitList() {
	var $orbit_select = $("select[name='orbit_select']");
	var option_orb = "<option value=\"0\" selected>All</option>";
	$orbit_select.html(option_orb);
	
	var satellite = $("select[name='satellite_select']").val();
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
				var a = 1;
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
	var chart = RadarChart.chart();
	
	var w = $(".statistics .reports-orbit").width() - 1;
	var h = $(".statistics .filters").height() - 2;
	var size = w > h ? h : w;
	
	$.when(getOrbitStatistics(satellite, siteId, orbit, fromDate, toDate))
	.done(function (response) {
		var header = "orbit";
		var body = orbit == "0" ? "acquisitions_all" : "acquisitions_" + orbit;
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
					if ( j == 0) {
						newSeries.className = v;
						newSeries.axes = [];
					} else {
						newSeries.axes.push({ "axis": [headers[j]], "value": parseFloat(v) });
					}
				});
				data.push(newSeries);
			}
		});
		RadarChart.defaultConfig.radius = 3;
		RadarChart.defaultConfig.w = size;
		RadarChart.defaultConfig.h = size;
		RadarChart.draw("#chart_orbit", data);
	})
	.fail(function (jqXHR, status, error) {
		alert(error);
	});
}
function showAggregate(satellite, siteId, orbit, fromDate, toDate) {
	$.when(getAggregateStatistics(satellite, siteId, orbit, fromDate, toDate))
	.done(function (response) {
		var pairs            = { "key": "Pairs"            , "color": "#4e4e4e", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var processed        = { "key": "Processed"        , "color": "#2eb32e", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var notYetProcessed  = { "key": "Not Yet Processed", "color": "#007bbb", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var falselyProcessed = { "key": "Falsely Processed", "color": "#ffc100", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var noIntersections  = { "key": "No Intersections" , "color": "#a5a5a5", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var errors           = { "key": "Errors"           , "color": "#bb0000", "type": "bar" , "yAxis": 1, "values" : [ ] };
		var acquisitions     = { "key": "Acquisitions"     , "color": "#336cc1", "type": "line", "yAxis": 2, "values" : [ ] };
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
			axisKeys.push(value.calendarDate.substring(5));
		});
		
		nv.addGraph(function() {
			var chart = nv.models.multiChart();
			chart.margin({ top: 0, right: 60, bottom: 20, left: 50 });
			//chart.useInteractiveGuideline(true);
			chart.xAxis
				.tickFormat(function (d) {
					return (typeof axisKeys[d] !== "undefined") ? axisKeys[d] : "";
				});
			
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
			
			$("#chart_aggregate svg").remove();
			$("#chart_aggregate").append("<svg style='width:100%;height:500px'> </svg>");
			d3.select("#chart_aggregate svg")
				.datum(seriesData)
				.call(chart);

			nv.utils.windowResize(chart.update);
			
			return chart;
		});
	})
	.fail(function (jqXHR, status, error) {
		alert(error);
	});
}

function updateFromToDate(repYear, month, last) {
	if ((month == "2") && (parseInt(repYear)/4 === parseInt(repYear/4))) {
		last = "29";
	}
	if (parseInt(month) < 10) {
		month = "0" + month;
	}
	$("input[name='fromDate']").val(repYear + "-" + month + "-" + "01");
	$("input[name='toDate']").val(repYear + "-" + month + "-" + last);
}
$(document).ready(function() {
	//$("select[name=satellite_select]").val("S1");
	
	var mH = $(".statistics .filters").get(0).offsetHeight;
	$(".statistics .reports-orbit").css({ "min-height": mH + "px" });
	
	var crtYear = (new Date().getFullYear() - 1 ) + "";
	$("input[name='report_year']").val(crtYear);
	var $selMonth = $("input[name='month_no']:checked");
	if ($selMonth.length > 0) {
		updateFromToDate(crtYear, $selMonth.val(), $selMonth.data("last"));
	}
	$("input[name='report_year']").on("change", function (e) {
		var $selMonth = $("input[name='month_no']:checked")
		if ($selMonth.length > 0) {
			if ($selMonth.get(0).id !== "custom_date") {
				updateFromToDate(this.value, $selMonth.val(), $selMonth.data("last"));
			}
		}
	});
	$("input[name='month_no']").on("change", function (e) {
		if (this.id != "custom_date") {
			var repYear = $("input[name='report_year']").val();
			if (repYear == "") {
				$("input[name='report_year']").val(new Date().getFullYear());
			}
			updateFromToDate($("input[name='report_year']").val(), this.value, $(this).data("last"));
		}
	});
	$("input[type='date']").on("change", function (e) {
		$("#custom_date").prop("checked", true);
	});
	$("select[name='satellite_select']").on("change", function (e) {
		if (this.value == "0") {
			$("input[name='update_orbit_report']").prop("disabled", true);
		} else {
			$("input[name='update_orbit_report']").prop("disabled", false);
		}
		updateOrbitList();
	});
	$("select[name='site_select']").on("change", function (e) {
		updateOrbitList();
	});
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
			var fromDate = $("input[name='fromDate']").val();
			var toDate   = $("input[name='toDate']").val();
			var orbit    = $("select[name='orbit_select']").val();
			var siteId   = $("select[name='siteId_select']").val();
			var sat      = $("select[name='satellite_select']").val();
			if (sat != "0") {
				$(".statistics .reports-orbit,.statistics .reports-aggregate").css({ "background-image": "none" });
				
				d3.selectAll("svg").selectAll("*").remove();
				d3.selectAll('.nvtooltip').remove();
				
				showRadar(sat, siteId, orbit, fromDate, toDate);
				showAggregate(sat, siteId, orbit, fromDate, toDate);
			}
		}
	});
});

//# sourceURL=statistics.js
</script>

<?php include 'ms_foot.php'; ?>
