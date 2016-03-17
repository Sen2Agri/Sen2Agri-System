<?php include 'master.php'; ?>
<?php
	//refresh page after 60 seconds
    $url1=$_SERVER['REQUEST_URI'];
    header("Refresh: 60; URL=$url1");
?>
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

$td1="<td colspan=\"2\">0</td>";
	$td2="<td colspan=\"2\">0</td>";
	$td3="<td colspan=\"2\">0</td>";
	while ( $row = pg_fetch_row ( $result_downloads ) ) {
		
		if($row[0] == 1){
		$td1 = "<td colspan=\"2\">" . $row [1] . "</td>";
		
		} else if($row[0] == 2){
			$td2 = "<td colspan=\"2\">" . $row [1] . "</td>";

		}else if($row[0] == 3){
			$td3 = 	"<td colspan=\"2\">" . $row [1] . "</td>";
		}
	}
	 echo  $td1.$td2.$td3;
}

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
							<table class="table full_width"">
								<thead>
									<tr>
										<th colspan="2">Downloads in progress</th>
										<th colspan="2">Successful downloads</th>
										<th colspan="2">Failed downloads</th>
									</tr>
								</thead>
							<!--  <tbody >-->
									<!-- <tr> -->
							<?php
							if (! (empty ( $_SESSION ['siteId'] ))) { // not admin
									?>
										<tbody >
										<tr id="refresh_info_user">
										<?php statistics_user();?>							
									
							     		</tbody>
							     		</tr>
							<?php } ?>
									
									<tr>
									<td colspan="2"></td>
									<td colspan="2"></td>
									<td colspan="2"></td></tr>
								<!-- </tbody>-->
							</table>
						</div>
						<!-------------- End Download statistics ---------------->

						<!-------------- Current processing details ---------------->
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
									<!--  <tbody  id="refresh_downloading">-->
									<tbody  id="refresh_downloading">
									
								<?php
								if (! (empty ( $_SESSION ['siteId'] ))) { // not admin
									current_downloads_user();
								}
									?>
									 
									 </tbody>
														
								<!--  </tbody>-->
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

<?php include 'ms_foot.php'; ?>