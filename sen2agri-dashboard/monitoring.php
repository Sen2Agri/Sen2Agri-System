<?php include 'master.php'; ?>

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
							<?php
							if (! (empty ( $_SESSION ['siteId'] ))) {
								$db = pg_connect ( 'host=' . ConfigParams::$SERVER_NAME . ' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
								
								$result = pg_query_params ( $db, "SELECT * FROM sp_get_sites($1)", array (
										$_SESSION ['siteId'] 
								) ) or die ( "Could not execute." );
								
								$option_site = "";
								while ( $row = pg_fetch_row ( $result ) ) {
									$option = "<option value=\"'.$row[0].'\">" . $row [1] . "</option>";
									$option_site = $option_site . $option;
								}
								echo $option_site;
							} else {
								$db = pg_connect ( 'host=' . ConfigParams::$SERVER_NAME . ' port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
								$sql = "SELECT * FROM sp_get_sites()";
								$result = pg_query ( $db, $sql ) or die ( "Could not execute." );
								
								$option_site = "<option value=\"\" selected>Select a site...</option>";
								while ( $row = pg_fetch_row ( $result ) ) {
									$option = "<option value=\"'.$row[0].'\">" . $row [1] . "</option>";
									$option_site = $option_site . $option;
								}
								echo $option_site;
							}
							?>
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
									<th colspan="2">Successful downloads:</th>
									<th colspan="2">Failed downloads:</th>
									<th colspan="2">In progress downloads:</th>
								</tr>
							</thead>
							<tbody>
								<tr>
									<td colspan="2">sdfsefs</td>
									<td colspan="2">sdfdsf</td>
									<td colspan="2">dfsdfdfd</td>
								</tr>
							</tbody>
						</table>
					</div>
					<!--------------End Download statistics ---------------->


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
										<th>Product</th>
										<th>Product Type(S2/L8)</th>
										<th>Status</th>
										<th>State modified time</th>
									</tr>
								</thead>
								<tbody>
									<tr>
										<td>sdfsefs</td>
										<td>sdfdsf</td>
										<td>dfsdfdfd</td>
										<td>156</td>
									</tr>
									<tr>
										<td></td>
										<td></td>
										<td></td>
										<td></td>
									</tr>
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
<?php include 'ms_foot.php'; ?>
