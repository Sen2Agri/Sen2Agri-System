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
	} else // if admin
{
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

function statistics_user() {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	
	$result_downloads = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history($1)", array (
			$_SESSION ['siteId'] 
	) ) or die ( "Could not execute." );
	
	$td_statistics = "";
	while ( $row = pg_fetch_row ( $result_downloads ) ) {
		$td = "<td colspan=\"2\">" . $row [0] . "</td>";
		$td_statistics = $td_statistics . $td;
	}
	echo $td_statistics;
}
/*
function statistics_admin(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	
	$result_downloads = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_downloader_history(null)", array () ) or die ( "Could not execute." );
	
	$td_statistics = "";
	while ( $row = pg_fetch_row ( $result_downloads ) ) {
		$td = "<td colspan=\"2\">" . $row [0] . "</td>";
		$td_statistics = $td_statistics . $td;
	}
	echo $td_statistics;
}
*/

/*
function current_downloads_admin(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_current_downloads(null)", array () ) or die ( "Could not execute." );

	$tr_current ="";
	while ( $row = pg_fetch_row ( $result ) ) {
		$tr = "<tr>".
				"<td colspan=\"2\">". $row[0]."</td>".
				"<td colspan=\"2\">". $row[1]."</td>".
				"<td colspan=\"2\">". $row[2]."</td>".
				"</tr>";
		$tr_current = $tr_current . $tr;
	}
	echo $tr_current;
	}
*/
	
function current_downloads_user(){
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );

	$result = pg_query_params ( $db, "SELECT * FROM sp_get_dashboard_current_downloads($1)", array (
			$_SESSION ['siteId']
	) ) or die ( "Could not execute." );
	
	$tr_current ="";
	while ( $row = pg_fetch_row ( $result ) ) {

		$tr = "<tr>".
				"<td colspan=\"2\">". $row[0]."</td>".
				"<td colspan=\"2\">". $row[1]."</td>".
				"<td colspan=\"2\">". $row[2]."</td>".
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
									onchange="siteInfo(this.value);siteInfoCurrent(this.value);">
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
							<table class="table full_width"">
								<thead>
									<tr>
										<th colspan="2">Downloads in progress</th>
										<th colspan="2">Successful downloads</th>
										<th colspan="2">Failed downloads</th>
									</tr>
								</thead>
								<tbody >
									<!-- <tr> -->
							<?php
							if (! (empty ( $_SESSION ['siteId'] ))) { // not admin
									?>
										<tr>
										<?php
										statistics_user();
							
										?></tr>
							<?php } else { // if admin ?>
										<tr  id="refresh_statistics">
										</tr>									
											
						<?php }
							?>
									<!-- </tr> -->
									<tr>
									<td colspan="2"></td>
									<td colspan="2"></td>
									<td colspan="2"></td></tr>
								</tbody>
							</table>
						</div>
						<!-------------- End Download statistics ---------------->


						<!-------------- Current processing details ---------------->

						<!--<input type="text" value="Current processing details"
						style="background: #f5f5f5; font-weight: bold; width: 200px; text-align: center"
						readonly>
					<div class="panel panel-default">  -->
						<div class="panel panel-default">
							<div class="panel-heading">Current downloads</div>
							<!-- <div class="panel-heading" style="background-color: #f5f5f5; font-weight: bold;">Current processing details</div> -->
							<div class="panel-body">
								<table class="table full_width" style="text-align: left">
									<thead>
										<tr>
											<th colspan="2">Product</th>
											<th colspan="2">Product Type</th>
											<th colspan="2">Status modified</th>
										</tr>
									</thead>
									<tbody  id="refresh_downloading">
								<?php
								
								if (! (empty ( $_SESSION ['siteId'] ))) { // not admin
								
									current_downloads_user();
						
								}else { // if admin
					
								}
									?>
						
								</tbody>
								</table>
							</div>
						</div>
						<!-------------- Current processing details ---------------->


				</div>
			</div>

			<!-- main -->
		</div>
	</div>
</div>

<script type="text/javascript">
//$("document").ready(function(){
	
function siteInfo(id) {
    $.ajax({
        type: "get",
        url: "getHistoryDownloads.php",
        data: {'siteID_selected' : id},
        success: function(result) {
             // alert(result);
        	$("#refresh_statistics").html(result);
        }
    });
}

function siteInfoCurrent(id) {
	$.ajax({
        type: "get",
        url: "getHistoryDownloadsCurrent.php",
        data: {'siteID_selected' : id},
        success: function(result) {
        	// alert(result);
        	$("#refresh_downloading").html(result);
        	// Schedule the next request
        }	
    });
}

//refresh current downloading every 120 seconds
/*var interval = setInterval(refresh_box(), 120000);
  function refresh_box() {

	  siteInfo(id);
	  siteInfoCurrent(id); 
  }*/
  
//refresh current downloading every 120 seconds
/*var interval = setInterval(refresh_box(), 120000);
  function refresh_box() {
    $("#refresh_statistics").load('getHistoryDownloads.php');
   // $("#refresh_downloading").load('getHistoryDownloads.php');
  }
  */
//});

</script>
<?php include 'ms_foot.php'; ?>