<?php include 'master.php'; ?>
<?php 
function select_option() {
	if (! (empty ( $_SESSION ['siteId'] ))) { // not admin
		$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
		$result = pg_query_params ( $db, "SELECT * FROM sp_get_sites($1)", array (
				$_SESSION ['siteId'] 
		) ) or die ( "Could not execute." );
		
		$option_site = "";
		while ( $row = pg_fetch_row ( $result ) ) {
			$option = "<option value=\"$row[0]\">" . $row [1] . "</option>";
			$option_site = $option_site . $option;
		}
		echo $option_site;
	} 
}

function statistics_user() {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

	$result_downloads = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history($1)", array (
			$_SESSION ['siteId']
	) ) or die ( "Could not execute." );

	$td1="<td>0</td>";
	$td2="<td>0</td>";
	$td3="<td>0</td>";
	while ( $row = pg_fetch_row ( $result_downloads ) ) {
		
		if($row[0] == 1){
			$td1 = "<td>" . $row [1] . "</td>";
		
		} else if($row[0] == 2){
			$td2 = "<td>" . $row [1] . "</td>";

		}else if($row[0] == 3){
			$td3 = 	"<td>" . $row [1] . "</td>";
		}
	}
	 echo  $td1.$td2.$td3;
}

function current_downloads_user(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	
	$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_current_downloads($1)", array ( $_SESSION ['siteId']) ) or die ( "Could not execute." );
	
	$tr_current ="";
	while ( $row = pg_fetch_row ( $result ) ) {
		$tr = "<tr>".
				"<td>". $row[0]."</td>".
				"<td>". $row[1]."</td>".
				"<td>". $row[2]."</td>".
			"</tr>";
		$tr_current = $tr_current . $tr;
	}
	echo $tr_current;
}
function get_history_jobs_user(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$result="";
	$result = pg_query_params ( $db, "SELECT * FROM sp_get_job_history($1, 1)", array ( $_SESSION ['siteId']) ) or die ( "Could not execute." );
	
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
								<select class="form-control" name="site_select">
									<!-- get sites -->
									<?php  select_option();?>	
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
									<?php if (! (empty ( $_SESSION ['siteId'] ))) { // not admin ?>
									<tr id="refresh_info_user">
										<?php statistics_user();?>							
									</tr>
									<?php } ?>
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
										
											<th>Product</th>
											<th>Product Type</th>
											<th>Status modified</th>
										</tr>
									</thead>
									<tbody id="refresh_downloading">
										<?php if (! (empty ( $_SESSION ['siteId'] ))) { current_downloads_user(); } ?>
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
										<?php get_history_jobs_user();?>
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
var timer3;
function siteJobs(page_no) {
<?php
if (! (empty ( $_SESSION ['siteId'] ))) {
	echo "id = " . $_SESSION['siteId'] . ";";
?>
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
        	timer3 =  setTimeout(siteJobs, 60000, page_no);
        }
    });
<?php } ?>
}

function move_to_first_jobs_page()
{
	id = $('select[name=site_select] option:selected').val();
	jsonJobsPage = 1;
	siteJobs(jsonJobsPage);
}

function move_to_previous_jobs_page()
{
	if (jsonJobsPage > 1) {
		id = $('select[name=site_select] option:selected').val();
		jsonJobsPage --;
		jsonJobsPage = jsonJobsPage < 1 ? 1 : jsonJobsPage;
		siteJobs(jsonJobsPage);
	} else {
		jsonJobsPage = 1;
	}
}

function move_to_next_jobs_page()
{
	if (!$("#page_move_next").hasClass("disabled")) {
		id = $('select[name=site_select] option:selected').val();
		jsonJobsPage ++;
		siteJobs(jsonJobsPage);
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