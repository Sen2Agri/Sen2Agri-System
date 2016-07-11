<?php include 'master.php'; ?>
<?php
if (! (empty ( $_SESSION ['siteId'] ))) { // not admin
		header("Location: monitoring_user.php");
		exit();
	}

function select_option() {
	if (empty ( $_SESSION ['siteId'] )) { 
		 // if admin
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$sql = "SELECT * FROM sp_get_sites()";
		$result = pg_query ( $db, $sql ) or die ( "Could not execute." );
		
		$option_site = "<option value=\"0\" selected>Select a site...</option>";
		while ( $row = pg_fetch_row ( $result ) ) {
			$option = "<option value=\"$row[0]\">" . $row [1] . "</option>";
			$option_site = $option_site . $option;
		}
		echo $option_site;
	}
}

function statistics_admin(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$result_downloads ="";
	$result_downloads = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history(null)", array () ) or die ( "Could not execute." );
	
	$table="";
	while ( $row = pg_fetch_row ( $result_downloads ) ) {

		$td = "<td>" . $row [1] . "</td>";
		$table = $table . $td;
	}
	echo $table;
}

function current_downloads_admin(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$result="";
	$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_current_downloads(null)", array () ) or die ( "Could not execute." );
	
	$tr_current ="";
	while ( $row = pg_fetch_row ( $result ) ) {
		$tr = "<tr>".
				"<td>". $row[0]."</td>".
				"<td>". $row[1]."</td>".
				"<td>". $row[2]."</td>".
				"<td>". $row[3]."</td>".
			"</tr>";
		$tr_current = $tr_current . $tr;
	}
	echo $tr_current;
}
function get_history_jobs_admin(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$result="";
	$result = pg_query_params ( $db, "SELECT * FROM sp_get_job_history(null, 1)", array () ) or die ( "Could not execute." );
	
	$tr_current ="";
	while ( $row = pg_fetch_row ( $result ) ) {
		$tr = "<tr>".
				"<td>". $row[0]."</td>".
				"<td>". $row[1]."</td>".
				"<td>". $row[2]."</td>".
				"<td>". $row[3]."</td>".
				"<td>". $row[4]."</td>".
				"<td>". $row[5]."</td>".
			"</tr>";
		$tr_current = $tr_current . $tr;
	}
	echo $tr_current;
}
?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<!-- main -->
			<div class="panel panel-default">
				<div class="panel-heading">Monitoring</div>
				<div class="panel-body">
					
					<div class="row">
						<div class="col-md-2">
							<!-- select site -->
							<div class="form-group form-group-sm">
								<select class="form-control" name="site_select"
										onchange="siteInfo(this.value); siteInfoCurrent(this.value); siteJobs(this.value, 1);">
									<?php select_option();?>
								</select>
							</div>
							<!-- end select site -->
						</div>
					</div>
					
					<!-------------- Download statistics ---------------->
						<div class="panel panel-default">
							<div class="panel-heading">Download statistics</div>
							<table class="table full_width">
								<thead>
									<tr>
										<th>Downloads in progress</th>
										<th>Successful downloads</th>
										<th>Failed downloads</th>
									</tr>
								</thead>
								<tbody >
								
									<tr id="refresh_statistics">
										<?php statistics_admin();?>
									</tr>
									
								</tbody>								
							</table>
						</div>
						<!-------------- End Download statistics ---------------->
						
						<!-------------- Current processing details ---------------->
						<div class="panel panel-default">
							<div class="panel-heading">Current downloads</div>
							<div class="panel-body">
								<table class="table full_width" style="text-align: left">
									<thead>
										<tr>
											<th>Site</th>
											<th>Product</th>
											<th>Product Type</th>
											<th>Status modified</th>
										</tr>
									</thead>
									<tbody id="refresh_downloading">
										<?php current_downloads_admin();?>
									</tbody>
								</table>
							</div>
						</div>
						<!-------------- Current processing details ---------------->
						
						<!-------------- Job history ---------------->
						<div class="panel panel-default">
							<div class="panel-heading">Current downloads</div>
							<div style="float: right; padding: 10px;">
								<button id="page_move_first" type="button" class="btn btn-default" onclick="move_to_first_jobs_page();">&lt;&lt;</button>
								<button id="page_move_prev"  type="button" class="btn btn-default" onclick="move_to_previous_jobs_page();">Previous Page</button>
								<button id="page_current"    type="button" class="btn btn-default" style="font-weight: 700;">1</button>
								<button id="page_move_next"  type="button" class="btn btn-default" onclick="move_to_next_jobs_page()">Next Page</button>
								<button id="page_move_last"  type="button" class="btn btn-default disabled" onclick="move_to_last_jobs_page();">&gt;&gt;</button>
							</div>
							<div class="panel-body">
								<table class="table full_width" style="text-align: left" id="history_jobs">
									<thead>
										<tr>
											<th>Job ID</th>
											<th>End timestamp</th>
											<th>Processor</th>
											<th>Site</th>
											<th>Status</th>
											<th>Start type</th>
										</tr>
									</thead>
									<tbody id="refresh_jobs">
										<?php get_history_jobs_admin();?>
									</tbody>
								</table>
							</div>
						</div>
						<!-------------- Job history ---------------->
				</div>
			</div>
			<!-- main -->
		</div>
	</div>
</div>

<script type="text/javascript">
var timer1;
var timer2;
var timer3;

function siteInfo(id) {
	$.ajax({
        type: "get",
        url: "getHistoryDownloads.php",
        data: {'siteID_selected' : id},
        success: function(result) {
          	$("#refresh_statistics").html(result);
        	 clearTimeout(timer1);
        	 timer1 = setTimeout(siteInfo, 5000, id);
        }
    });
}

function siteInfoCurrent(id) {
	$.ajax({
        type: "post",
        url: "getHistoryDownloadsCurrent.php",
		data:  {'siteID_selected' : id}, 
        success: function(result) {
        	$("#refresh_downloading").html(result);
        	// Schedule the next request
     		clearTimeout(timer2);
        	timer2 =  setTimeout(siteInfoCurrent, 5000, id);
        }
    });
}

function siteJobs(id, page_no) {
	jsonJobsPage = page_no;
	$.ajax({
        type: "post",
        url: "getHistoryJobs.php",
		data:  { 'siteID_selected': id, 'page': page_no }, 
        success: function(result) {
        	$("#refresh_jobs").html(result);
        	
			// display page number
			$("#page_current").html(page_no);
			// toggle move_next button availability
			if ($("#history_jobs").children('tbody').children('tr').length < 20) {
				$("#page_move_next").addClass("disabled");
			} else {
				$("#page_move_next").removeClass("disabled");
			}
			
			// Schedule the next request
     		clearTimeout(timer3);
        	timer3 =  setTimeout(siteJobs, 5000, id, page_no);
        }
    });
}

function move_to_first_jobs_page()
{
	id = $('select[name=site_select] option:selected').val();
	jsonJobsPage = 1;
	siteJobs(id, jsonJobsPage);
}

function move_to_previous_jobs_page()
{
	if (jsonJobsPage > 1) {
		id = $('select[name=site_select] option:selected').val();
		jsonJobsPage --;
		jsonJobsPage = jsonJobsPage < 1 ? 1 : jsonJobsPage;
		siteJobs(id, jsonJobsPage);
	} else {
		jsonJobsPage = 1;
	}
}

function move_to_next_jobs_page()
{
	if (!$("#page_move_next").hasClass("disabled")) {
		id = $('select[name=site_select] option:selected').val();
		jsonJobsPage ++;
		siteJobs(id, jsonJobsPage);
	}
}
$( document ).ready(function() {
	// toggle move_next button availability
	if ($("#history_jobs").children('tbody').children('tr').length < 20) {
		$("#page_move_next").addClass("disabled");
	} else {
		$("#page_move_next").removeClass("disabled");
	}
});
</script>

<?php include 'ms_foot.php'; ?>