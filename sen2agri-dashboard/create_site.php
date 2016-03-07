<?php include 'master.php'; ?>
<?php 
//processing submitted dates
if(isset($_REQUEST['add_site']) && $_REQUEST ['add_site'] == 'AddSite'){

	// first character to uppercase.
	$site_name = ucfirst($_REQUEST['sitename']);
	
	//coord geog from the upload file
	//'POLYGON((35.940406 54.140187, 37.651121 54.140187, 37.651121 53.123645, 35.940406 53.123645, 35.940406 54.140187))'
	$coord_geog = "POLYGON((35.940406 54.140187, 37.651121 54.140187, 37.651121 53.123645, 35.940406 53.123645, 35.940406 54.140187))";
	$winter_start = "";
	$winter_end = "";
	$summer_start = "";
	$summer_end = "";
	
	//make site short name
	$site_short_name =str_replace(")","",str_replace(array(" ", "("),"_",strtolower($site_name)));
	//echo $site_short_name;
	
	//keep only day and month, ex:3002
	function dayMonth($param){
		$date = date('dm',strtotime($param));
		return $date;
	}
	//insert new site into database
	function insertSiteSeason($site,$short_name,$coord,$wint_start,$wint_end,$summ_star,$summ_end){
		$db = pg_connect ( 'host=sen2agri-dev port=5432 dbname=sen2agri user=admin password=sen2agri' ) or die ( "Could not connect" );
		$sql="SELECT sp_dashboard_add_site($1,$2,$3,$4,$5,$6,$7)";
		$res = pg_prepare ( $db, "my_query", $sql );
	
		$res = pg_execute ( $db, "my_query", array (
				$site,
				$short_name,
				$coord,
				$wint_start,
				$wint_end,
				$summ_star,
				$summ_end
		) ) or die ("An error occurred.");
		
	//	echo "it works";
	}

	
	if(isset($_REQUEST['season']))
		// winter season selected
		if ($_REQUEST['season'] =="0"){
		$winter_start = dayMonth($_REQUEST['startseason_winter']);
		$winter_end = dayMonth($_REQUEST['endseason_winter']);		
		insertSiteSeason($site_name,$site_short_name,$coord_geog,$winter_start,$winter_end,$summer_start,$summer_end);
		
	} else
		// summer season selected
		if($_REQUEST['season'] =="1"){
		$summer_start = $_REQUEST['startseason_summer'];
		$summer_end = $_REQUEST['endseason_summer'];
		insertSiteSeason($site_name,$site_short_name,$coord_geog,$winter_start,$winter_end,$summer_start,$summer_end);
		
	}else 
		//summer and winter season selected
		if ($_REQUEST['season'] =="2"){
			$winter_start = $_REQUEST['startseason_winter'];
			$winter_end = $_REQUEST['endseason_winter'];
			$summer_start = $_REQUEST['startseason_summer'];
			$summer_end = $_REQUEST['endseason_summer'];
			
			insertSiteSeason($site_name,$site_short_name,$coord_geog,$winter_start,$winter_end,$summer_start,$summer_end);
	}
	

}

?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<!-- Start code for adding site---------- -->
			<div class="panel panel-default">
				<div class="panel-heading">Create new site</div>
				<div class="panel-body">
					<!-- form -->
					<form role="form" id="siteform" action="create_site.php"
						method="post">
						<div class="row">
							<div class="col-md-1"></div>
							<div class="col-md-3">

								<div class="form-group  form-group-sm">
									<label class="control-label" for="sitename">Site name:</label>
									<input type="text" class="form-control" id="sitename"
										name="sitename">
								</div>

								<div class="form-group form-group-sm">
									<label class="control-label" for="season"> Season: <select
										id="season" class="form-control" name="season"
										onchange="displaySeason()">
											<option value="0" selected>Winter</option>
											<option value="1">Summer</option>
											<option value="2">Winter and Summer</option>
									</select>

									</label>
								</div>


								<div id="div_startseason1" class="form-group form-group-sm">
									<label class="control-label" for="startseason_winter">Winter season from:</label>
									<input type="text" class="form-control" id="startseason_winter"
										name="startseason_winter">
								</div>

								<div id="div_endseason1" class="form-group form-group-sm">
									<label class="control-label" for="endseason_winter">to:</label>
									<input type="text" class="form-control" id="endseason_winter"
										name="endseason_winter">
								</div>



								<div id="div_startseason2" class="form-group form-group-sm"
									style="display: none">
									<label class="control-label" for="startseason_summer">Summer season from:</label>
									<input type="text" class="form-control" id="startseason_summer"
										name="startseason_summer">
								</div>

								<div id="div_endseason2" class="form-group form-group-sm"
									style="display: none">
									<label class="control-label" for="endseason_summer">to:</label>
									<input type="text" class="form-control" id="endseason_summer"
										name="endseason_summer">
								</div>

								<div class="form-group form-group-sm">
									<label class="control-label" for="siteupload">Upload site shape file:</label>
									<input type="file" class="form-control" id=""siteupload""
										name=""siteupload"">
								</div>


								<input class="btn btn-primary " name="add_site" type="submit"
									value="AddSite">


							</div>
						</div>
					</form>
					<!--end  form -->

				</div>
			</div>


			<!-- End code for adding site---------- -->
		</div>
	</div>
</div>



<!-- includes for  datepicker-->
<link rel="stylesheet"
	href="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/themes/smoothness/jquery-ui.css">
<script
	src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
<script
	src="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.4/jquery-ui.min.js"></script>
<!-- end includes for  datepicker-->

<!--Jquery datepicker -->
<script>
		$(document).ready(function() {
			$( "#startseason_winter" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0,//past dates disable from current date
				 onSelect: function(selected) {
				    $("#endseason_winter").datepicker("option","minDate", selected)
					 	        }
					  });
			$( "#endseason_winter" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0
					  });
			  
			$( "#startseason_summer" ).datepicker({
				 dateFormat: "yy-mm-dd" ,
				 minDate: 0,
				 onSelect: function(selected) {
				    $("#endseason_summer").datepicker("option","minDate", selected)
					 	        }
					  });
			$( "#endseason_summer" ).datepicker({
				 dateFormat: "yy-mm-dd",
				 minDate: 0 
					  });
		});
</script>
<!--end Jquery datepicker -->

<!-- validate form  -->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>
<script>
	$(document).ready( function() {
							$("#siteform").validate(
											{
												rules : {
													sitename:"required",						
													startseason_winter : "required",
													endseason_winter:"required",
													startseason_summer:"required",
													endseason_summer:"required"/*,
													siteupload:"required"*/
												},
												
												// the errorPlacement has to take the table layout into account
												errorPlacement : function(
														error, element) {
													error.appendTo(element
															.parent());
												},

												submitHandler :function(form) {
													$.ajax({
											        url: $(form).attr('action'),
											        type: $(form).attr('method'),
											        data: $(form).serialize(),
											        success: function(response) {
									                    alert("Your form was submitted!");
									                   //clear inputs after submit
									                   $("#siteform")[0].reset();		
											                 }     
											         });
											 },
												// set this class to error-labels to indicate valid fields
												success : function(label) {
													label.remove();
												},
												highlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.addClass(
																	"has-error");
												},
												unhighlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.removeClass(
																	"has-error");
												}
											});
	});
</script>
<!-- end validate -->



<!-- Select season event -->
<script type="text/javascript">
function displaySeason(){
	var elem = document.getElementById("season").value;
	if(elem =="0"){
		document.getElementById("div_startseason1").style.display = "block";
		document.getElementById("div_endseason1").style.display = "block";

		document.getElementById("div_startseason2").style.display = "none";
		document.getElementById("div_endseason2").style.display = "none";
		}
	else if(elem =="1"){
		document.getElementById("div_startseason1").style.display = "none";
		document.getElementById("div_endseason1").style.display = "none";
		
		document.getElementById("div_startseason2").style.display = "block";
		document.getElementById("div_endseason2").style.display = "block";
		}
	else if(elem =="2"){
		document.getElementById("div_startseason1").style.display = "block";
		document.getElementById("div_endseason1").style.display = "block";
		
		document.getElementById("div_startseason2").style.display="block";
		document.getElementById("div_endseason2").style.display="block";
		}
}
</script>
<?php include 'ms_foot.php'; ?>