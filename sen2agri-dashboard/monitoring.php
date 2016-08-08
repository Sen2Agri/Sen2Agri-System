<?php include 'master.php'; ?>
<?php
function select_option() {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	if (empty ( $_SESSION ['siteId'] )) {
		// admin
		$sql         = "SELECT * FROM sp_get_sites()";
		$result      = pg_query ( $db, $sql ) or die ( "Could not execute." );
		$option_site = "<option value=\"0\" selected>Select a site...</option>";
	} else {
		// not admin
		$sql         = "SELECT * FROM sp_get_sites($1)";
		$result      = pg_query_params ( $db, $sql, array ($_SESSION ['siteId']) ) or die ( "Could not execute." );
		$option_site = "";
	}
	while ( $row = pg_fetch_row ( $result ) ) {
		$option = "<option value=\"$row[0]\">" . $row [1] . "</option>";
		$option_site = $option_site . $option;
	}
	echo $option_site;
}
?>

<div id="main">
	<div id="main2">
		<div id="main3">
			<!-- Select Site -->
			<div class="row">
				<div class="col-md-2">
					<div class="form-group form-group-sm">
						<select class="form-control" name="site_select" onchange="siteInfo(); siteInfoCurrent(); siteJobs(1);">
							<?php select_option();?>
						</select>
					</div>
				</div>
			</div>
			<!-- End Select Site -->
			
			<!-------------- Download statistics ---------------->
			<div class="panel panel-default config for-table">
				<div class="panel-heading"><h4 class="panel-title"><span>Download statistics</span></h4></div>
				<div class="panel-body">
					<table class="table table-striped">
						<thead>
							<tr>
								<th>Downloads in progress</th>
								<th>Successful downloads</th>
								<th>Failed downloads</th>
							</tr>
						</thead>
						<tbody >
							<tr id="refresh_statistics">
							</tr>
						</tbody>
					</table>
				</div>
			</div>
			
			<!-------------- Current processing details ---------------->
			<div class="panel panel-default config for-table">
				<div class="panel-heading"><h4 class="panel-title"><span>Current downloads</span></h4></div>
				<div class="panel-body">
					<table class="table table-striped" style="text-align: left">
						<thead>
							<tr>
								<th>Site</th>
								<th>Product</th>
								<th>Product Type</th>
								<th>Status modified</th>
							</tr>
						</thead>
						<tbody id="refresh_downloading">
						</tbody>
					</table>
				</div>
			</div>
			
			<!-------------- Job history ---------------->
			<div class="panel panel-default config for-table">
				<div class="panel-heading"><h4 class="panel-title"><span>Jobs history</span></h4></div>
				<div class="panel-body">
					<div class="pagination">
						<label class="rows_per_page" for="rows_per_page">Rows/page:
							<select class="rows_per_page" name="rows_per_page" id="rows_per_page" onchange="pagingGo(1);">
								<option value="10" selected>10</option>
								<option value="20">20</option>
								<option value="50">50</option>
							</select>
						</label>
						<ul class="pages"></ul>
						<div class="gotobox hidden" id="gotobox">
							<div class="uparrow"></div>
							<div class="text">Go to page:</div> 
							<input type="text" maxlength="3" name="gotobox-input" id="gotobox-input" value=""/>
							<a href="javascript:;" class="smbutton-go smbutton-green" onclick='pagingGoTo();'>Go</a>&nbsp;
						</div>
					</div>
					<table class="table table-striped" style="text-align: left" id="history_jobs">
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
						</tbody>
					</table>
				</div>
			</div>
			<!-------------- Job history ---------------->
		</div>
	</div>
</div>
<script src="scripts/pager/pager.js"></script>
<script type="text/javascript">
	var timer1;
	var timer2;
	
	function siteInfo() {
		var id = $('select[name=site_select] option:selected').val();
		$.ajax({
			type: "get",
			url: "getHistoryDownloads.php",
			data: {'siteID_selected' : id},
			success: function(result) {
				$("#refresh_statistics").html(result);
				 clearTimeout(timer1);
				 timer1 = setTimeout(siteInfo, 10000);
			}
		});
	}
	
	function siteInfoCurrent() {
		var id = $('select[name=site_select] option:selected').val();
		$.ajax({
			type: "post",
			url: "getHistoryDownloadsCurrent.php",
			data:  {'siteID_selected' : id}, 
			success: function(result) {
				$("#refresh_downloading").html(result);
				// Schedule the next request
				clearTimeout(timer2);
				timer2 = setTimeout(siteInfoCurrent, 10000);
			}
		});
	}
	
	function siteJobs(page_no) {
		var id = $('select[name=site_select] option:selected').val();
		var rows_per_page = $('select[name=rows_per_page] option:selected').val();
		$.ajax({
			type: "post",
			url:  "getHistoryJobs.php",
			data:  { 'siteID_selected': id, 'page': page_no, 'rows_per_page': rows_per_page }, 
			success: function(result) {
				// refresh pagination: set count & redraw
				var count_rows = result.substring(4, result.indexOf("-"+"->"));
				pagingRefresh(count_rows, page_no);
				// refresh jobs history table
				$("#refresh_jobs").html(result);
			}
		});
	}
	
	$( document ).ready(function() {
		siteInfo();
		siteInfoCurrent();
		siteJobs(1);
	});

	// Actions to be performed when page changes
	var orig_pagingGo = window.pagingGo;
	window.pagingGo = function(page_no) {
		orig_pagingGo(page_no);
		siteJobs(page_no);
	}
</script>

<?php include 'ms_foot.php'; ?>