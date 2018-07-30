<?php include "master.php"; ?>
<?php 

$dbconn       = pg_connect(ConfigParams::getConnection()) or die ("Could not connect");
$sites_id = sizeof(ConfigParams::$SITE_ID)>0? '{'.implode(',',ConfigParams::$SITE_ID).'}':null;

$sites = array();
$option_site = "";
if( sizeof(ConfigParams::$SITE_ID)>0 || $_SESSION['isAdmin']){
    $rows = pg_query_params ( $dbconn, "select * from sp_get_products_sites($1)", array ($sites_id)) or die(pg_last_error());

    while ( $row = pg_fetch_row ( $rows ) ) {
    	$sites[] = $row [0];
    	$option = "<option value='" . $row [0] . "'>" . $row [1] . "</option>";
    	$option_site = $option_site . $option;
    }
}

$sql ='SELECT * FROM sp_get_product_types() ';
$result = pg_query ( $dbconn, $sql ) or die ( "Could not execute." );
$option_product_type = "";
while ( $row = pg_fetch_row ( $result ) ) {
    if($row [0] != "7"){//if product type is not L1C 
		$option = "<option value='" . $row [0] . "'>" . $row [1] . "</option>";
		$option_product_type .= $option;
    }
}

?>
    <div id="main">
        <div id="main2">
            <div id="main3">
        		<div style="margin-bottom: 2px;">
        		<button type="button" class="btn btn-default btn-xs" onclick="applyFilter()">
		          <span class="glyphicon glyphicon-filter" style="color:#6F8D33"></span> Filter 
		        </button>
		        
		        <button type="button" class="btn btn-default btn-xs" id="btnResetFilter" disabled>
		          <span class="glyphicon glyphicon-refresh" style="color:#337AB7"></span> Reset Filter 
		        </button>
		        </div>		 
				<table cellpadding="0px" cellspacing="0px">
					<tr>
						<td><div id="tree" class="tree"></div></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>
						<td><div id="map" class="map"></div></td>
					</tr>
				</table>
           
      			<!------------------------------ Dialog Apply Filter -------------------------------->
      			<div class="filter-product-site" id="div_filter" style="display: none;">
                	<form enctype="multipart/form-data" id="apply_filter" action="" method="post">
                	
                    	 <!-- List of sites -->
                         <div class="row" id="div_site" required>
                         	<div class="col-md-3">
                            	<label class="control-label" for="choose_site">Site name:</label>                                	
                            </div>          
                            <div>
                            	<select id="choose_site" name="choose_site">
									<option value="" >Select site</option>			
									<?= $option_site ?>					
								</select>
                            </div>
                         </div>
                         
                         <!-- Sensor -->                      
                          <div class="row" id="div_sensor">
                         	<div class="col-md-3">
                               	<label>Sensor:<span style="color:red">*</span></label>                                                        
                            </div>
                            <div>
                            <?php if(!ConfigParams::isSen2Agri()){?>
                            	<label class="checkbox-inline" style="margin-bottom: 5px" for="sensor"><input type="checkbox" id="S1" name="sensor" value="1" checked>S1</label>
                            <?php }?>
                            	<label class="checkbox-inline" style="margin-bottom: 5px" for="sensor"><input type="checkbox" id="S2" name="sensor" value="1" checked>S2</label>
                                <label class="checkbox-inline" style="margin-bottom: 5px" for="sensor"><input type="checkbox" id="L8" name="sensor" value="2" checked>L8</label>                                                       
                            </div>                           
                         </div>
                         <div class="row errorClass" style="display:none;">
                         	<div class="col-md-3" >
                            </div>
                            <div>Sensor field is required.</div>
                         </div>
                       
                         
                         <!-- Product Type -->                      
                          <div class="row" id="div_product" required>
                         	<div class="col-md-3">
                               	<label>Product Type:</label></div>
                            <div >
                            	<select id="product_type"  multiple="multiple">
                            		<?= $option_product_type?>                            	  
                            	</select>                             
                            </div>
                         </div>
                         
                          <!-- Tiles -->
						  <div class="row" id="div_tiles" >
                         	<div class="col-md-3"><label for="tiles">Tiles:</label></div>
	                        <div class=" ui-widget">
	                        	<input type="text" id="tiles" class="schedule_format" disabled>								         
                            </div>
                         </div>
                         
                        <!-- Season -->
                        <div class="row" >
                         	<div class="col-md-4">
                         		<label class="radio-inline"><input type="radio" id="optseason" name="optradio" value="season" style="margin-top: 2px;font-weigth:bold" disabled>Filter by season</label>
                            </div>
                        </div>
                        <div class="row">
                        	<div class="col-md-1"></div>
                         	<div class="col-md-2">
                            	<label class="control-label">Season:</label></div>
                            <div>
                                <select id="choose_season" name="choose_season" disabled>
									<option value="" >Select season</option>									
								</select>                   			 
                            </div>
                         </div> 
                         
                         <!--  Period of time -->
                         <div class="row" id="div_filterByInterval">
                         	<div class="col-md-4">
                         		<label class="radio-inline"><input type="radio" id="optinterval" name="optradio" value="interval" style="margin-top: 2px;font-weigth:bold" checked>Filter by interval</label>
                            </div>
                         </div>
                         <div class="row ">
                         	<div class="col-md-1"></div>
                         	<div class="col-md-2">                         		
                            	<label class="control-label">From:</label>
                            </div>
                            <div>
                       			<input type="text" id="startdate" name="startdate"  class="schedule_format startdate">                           			 
                            </div>
                         </div>
                         <div class="row div_filterByInterval">
                         	<div class="col-md-1"></div>
                         	<div class="col-md-2">                         		
                           		<label class="control-label">To:</label>
                            </div>
                            <div>
                       			<input type="text" id="enddate" name="enddate" class="schedule_format enddate">                         			 
                            </div>
                         </div> 
                        
                        <!-- Buttons -->
                         <div class="row ">
                         	<div class="col-md-3"></div>            
	                        <div class="submit-buttons">
	                        	<input class="filter-product-site-btn" name="apply_filter_submit" type="button" value="Apply">
	                            <input class="filter-product-site-btn" name="abort_applyFilter" type="button" value="Cancel" onclick="abortApplyFilter()">
	                        </div>
                         </div>
                    </form>
                </div>
                
     
                <!------------------------------ End Dialog Apply Filter -------------------------------->
 			</div>
        </div>
    </div>

    <div id="wait">
     <span class="glyphicon glyphicon-refresh glyphicon-spin"></span>
    <span >Loading...</span></div>
    
    <link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
	<script src="libraries/jquery-ui/jquery-ui.min.js"></script>
	<script src="libraries/jquery-ui/jquery.ui.autocomplete.scroll.min.js"></script>
	<!-- includes for validate form -->
	<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
	<script src="libraries/jquery-validate/additional-methods.min.js"></script>
	
   
	<script src="libraries/bootstrap-3.3.6/js/bootstrap-multiselect.js"></script>
	<link rel="stylesheet" href="libraries/bootstrap-3.3.6/css/bootstrap-multiselect.css">	
	
	
    <script src="libraries/openlayers/build/ol.js"></script>
	<script>
	function initMultiSelect(){
		 $("#product_type").multiselect({
	           // enableFiltering: true,
	            includeSelectAllOption: true,
	            maxHeight: 200,
	            buttonWidth: '180px',
	            nonSelectedText: 'Select product type'	           
	          //  numberDisplayed: 2
	           // dropUp: true
			 });
	}

	 String.prototype.rtrim = function (s) {
	  	    if (s == undefined)
	  	        s = '\\s';
	  	    return this.replace(new RegExp("[" + s + "]*$"), '');
	  	};
	
	$( function() {
		 $(document).ajaxStart(function(){
		        $("#wait").css("display", "block");
		    });
		 $(document).ajaxComplete(function(){
		        $("#wait").css("display", "none");
		    });
		
		initMultiSelect();

		$('input[name="startdate"]').datepicker({dateFormat: "yy-mm-dd"});
    	$('input[name="enddate"]').datepicker({dateFormat: "yy-mm-dd"});
				 		 
		// create dialog for edit site form
	    $("#div_filter").dialog({
	        title: "Apply filter",
	        width: '450px',
	        autoOpen: false,
	        modal: true,
	        resizable: false
	    });

		$("input[name='sensor']").change(function(){
			$(".errorClass").css("display","none");
			});
		
	    $('#optseason').click(function(){
		    //make seasons list available
	    	$('#choose_season').removeAttr("disabled");

		    //disable inputs for interval
	    	$('input[name="startdate"]').attr('disabled',true);
	    	$('input[name="enddate"]').attr('disabled',true);
			});
	    
	    $('#optinterval').click(function(){
		    //disable seasons list
	    	$('#choose_season').attr('disabled',true);

	    	$('input[name="startdate"]').removeAttr("disabled");
	    	$('input[name="enddate"]').removeAttr("disabled");
			});
	    
		$('#choose_site').on('change',function(){		
			if($(this).val()==""){
				$('#optseason').attr('disabled',true);
				$('#choose_season').attr('disabled',true);
				$('#tiles').attr('disabled',true);
			}else{

				$('#optseason').removeAttr("disabled");
				//$('#choose_season').removeAttr("disabled");
				$('#tiles').removeAttr("disabled");

				$('#choose_season').find('option').not(':first').remove();
					
				//get seasons for selected list
				$.ajax({
					type: "POST",
	                url: "getTiles.php",
	                dataType: "json",	                   
	                data: {action: 'get_site_seasons',site_id:$(this).val()},
	                success: function(data){                   	
	                    	
	                	for(var i=0;i<data.length;i++){	    
	                    	var option = $("<option></option>");                		
	        		           option.val(data[i][0]);
	        		           option.text(data[i][1]);
	        		            
	        		           $('#choose_season').append(option);		
	                    }
	                   }
					});
				}

		});

		// functions needed by autocomplete
	    function MySplit( val ) {
		   return val.split( /,\s*/ );
		}
		function extractLast( term ) {
		   return MySplit( term ).pop();
		}

		Array.prototype.diff = function(a) {
		    return this.filter(function(i) {return a.indexOf(i) < 0;});
		};

		var availableTiles = [];
		$( "#tiles" )
		  // don't navigate away from the field on tab when selecting an item
		  .on( "keydown", function( event ) {
		      if ( event.keyCode === $.ui.keyCode.TAB &&
		        $( this ).autocomplete( "instance" ).menu.active ) {
		        event.preventDefault();
		      }
		  })
		  .autocomplete({
 		     minLength: 2,
 		     maxShowItems: 7,// Make list height fit to 7 items when items are over 7. 
		    source: function( request, response ) {
						var product = new Array();
						var productSelected = false;

						var sensors = $("input:checkbox[name='sensor']");
						$.each(sensors, function(index, sensor){
							if( $(sensor).is(':checked')){
								//push sensor id
				  	   	  		product.push($(sensor).val());
				  	   	  		productSelected = true;
					  	   	  }
							});
						
			  	   		var site_id = $('#choose_site').find(":selected").val();

			  	   		if(site_id != "" && productSelected == true){

			        	    $.ajax({
			        	    	type: "POST",
			                    url: "getTiles.php",
			                    dataType: "json",		                    
			                    data: {action: 'get_tiles',site_id:site_id,satellite_id:product,term:extractLast( request.term )},
			                    success: function(data){
			                        availableTiles = data.diff(MySplit(request.term));
			           
			                    	response( $.ui.autocomplete.filter(
			                    			availableTiles, extractLast( request.term ) ) );
				                   }
			        	    });
			  	   		}else{ 
				  	   		if(site_id == ""){				  	   			
			  	   				$('#choose_site').addClass('fieldRequired');			  	   	  		 	
				  	   		}else if(productSelected == false){				  	   	
				  	   			$(".errorClass").css("display","");
					  	   	}
			  	   		}
		  		 },
		     focus: function() {
		            // prevent value inserted on focus
		            return false;
		          },
		     select: function( event, ui ) {		    	   
			            var terms = MySplit( this.value );
			            // remove the current input
			            terms.pop();
			            // add the selected item
			            terms.push( ui.item.value );
			            // add placeholder to get the comma-and-space at the end
			            terms.push( "" );
			            this.value = terms.join( ", " );

		            return false;
		          },
		
		  });
	    
});

	
	function abortApplyFilter(){
	   $("#div_filter").dialog("close");
	}	

	//reset filters
	$('#btnResetFilter').click(function(){
		dataFiltered = {};
		$.ajax({
			url: "getProductTreeData.php",
			type: "get",		
			cache: false,
			crosDomain: true,
			dataType: "json",
			success: function(data)
			{
				treeData = data;
				renderTree();
				$(".list-group").css("width", $(".tree").width() - 20);
				
				//make unavailable the button to reset the filter
				$('#btnResetFilter').prop('disabled',true);

				//reset form from filter dialog
				$("#apply_filter")[0].reset();
				$('#optseason').attr('disabled',true);
				$('#choose_season').attr('disabled',true);

				// reset multiselect after dialog close
				$("#product_type").multiselect( 'destroy' );
				initMultiSelect();
			},
			error: function (responseData, textStatus, errorThrown) {
				console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
				treeData = [];
				renderTree();
			}
		});
	});

	var dataFiltered = {};
	$("input[name='apply_filter_submit']").click(function() { 
		var error = true;
		var satellit_id = new Array();

		// check if sensor is selected
		var sensors = $("input:checkbox[name='sensor']");
		$.each(sensors, function(index, sensor){
			if( $(sensor).is(':checked')){
				error = false;
				$(".errorClass").css("display","none");

				//push sensor id
				satellit_id.push($(sensor).val());
  	   	        dataFiltered.satellit_id = satellit_id ;
	  	   	  }
			});
	
		if(error == false){			
			dataFiltered = {};
			
			var site_id = $('#choose_site').find(":selected").val();
			if(site_id!="") dataFiltered.site_id= site_id ;
			
	  	    var product_id = $('#product_type').val();
	  	    if(product_id!=null) dataFiltered.product_id= product_id ;

	  	    if($("#optseason").is(':checked')){
		  	    var season_id = $('#choose_season').find(":selected").val();
		  	    if(season_id!="") dataFiltered.season_id= season_id ;
	  	    }
	  	    
	  	 	if($("#optinterval").is(':checked')){
		  	    var start_data = $('#startdate').val();
		  	    if(start_data!="") dataFiltered.start_data= start_data ;
	
		  	    var end_data = $('#enddate').val();
		  	    if(end_data!="") dataFiltered.end_data= end_data ;
	  	 	}
	  	  	  	    
	  	    var tiles = $('#tiles').val();   	  	  
	  	    if(tiles!="")dataFiltered.tiles = tiles.rtrim(', ').split(', ');
	  		
			//recharge tree view
			$.ajax({
				url: "getProductTreeData.php",
				type: "post",
				data: dataFiltered,
				cache: false,
				crosDomain: true,
				dataType: "json",
				success: function(data)
				{
					treeData = data;
					renderTree();
					$(".list-group").css("width", $(".tree").width() - 20);

					//close filter dialog
					abortApplyFilter();
					
					//make available the button to reset the filter
					$('#btnResetFilter').prop('disabled',false);		
					
					if(data.length == 0){//if no result returned display info dialog
						var $dialog = $('<div></div>')	
							    .dialog({
							        width: 450,
							        title: 'Info'});

						$dialog.dialog('open');

						$dialog.html('No data found.');
						
						}
				},
				error: function (responseData, textStatus, errorThrown) {
					console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
					treeData = [];
					renderTree();
				}
			});

			
		}else {
			 $(".errorClass").css("display","");
			 }
	});

	function applyFilter(){
		$("#div_filter").dialog("open");
	}
	
		var styleSelected = new ol.style.Style({
			stroke: new ol.style.Stroke({
				color: 'rgba(255, 0, 0, 0.75)',
				lineDash: [4],
				width: 2
			}),
			fill: new ol.style.Fill({
				color: 'rgba(0, 0, 255, 0.01)'
			})
		});
		var vector = new ol.layer.Vector({
			source: new ol.source.Vector(),
			style: styleSelected
		});
		vector.setZIndex(10);

		var view = new ol.View({ center: [0, 0], zoom: 3 });
		//var raster = new ol.layer.Tile({ source: new ol.source.Stamen({layer: 'watercolor'}) });
		var raster = new ol.layer.Tile({ source: new ol.source.Stamen({layer: 'terrain'}) });

		var map = new ol.Map({
		  layers: [raster, vector],
		  target: 'map',
		  view: view
		});
		var lat = 40;
		var lon = 22;
		map.getView().setCenter(ol.proj.transform([lon, lat], 'EPSG:4326', 'EPSG:3857'));
		map.getView().setZoom(3);

		// Create an image layer
		var imageLayer;
		var displayProductImage = function(imageUrl,/* imageWidth, imageHeight,*/ extent) {
			if(imageLayer) map.removeLayer(imageLayer);
			if(vectorLayer) {
				map.removeLayer(vectorLayer);
			}
			

			if(!imageUrl) return;
			imageLayer = new ol.layer.Image({
				opacity: 0.75,
				source: new ol.source.ImageStatic({
					attributions: [
						new ol.Attribution({
							html: ''
						})
					],
					url: imageUrl,
					projection: map.getView().getProjection(),
					imageExtent: extent
				})
			});

			map.addLayer(imageLayer);
		};


		// Create a vector layer
		var vectorLayer;
		var displayProductVector = function(productUrl) {
			if(vectorLayer) {
				map.removeLayer(vectorLayer);
			}
			if(imageLayer) map.removeLayer(imageLayer);

			if(!productUrl) return;
			vectorLayer =  new ol.layer.VectorTile({
				source: new ol.source.VectorTile({
		            format: new ol.format.MVT({
		            	  geometryName: 'myGeom',
		            	  layerName: 'myLayer'
		            	}),
		            tileGrid: ol.tilegrid.createXYZ({maxZoom: 22}),
		            tilePixelRatio: 16,		           
					url: productUrl,
					attributions: [
						new ol.Attribution({
							html: ''
						})
					]
				}),
		        style: styleSelected,
		        projection: map.getView().getProjection()
			});

			map.addLayer(vectorLayer);
		};

		// select interaction working on "singleclick"
		var selectSingleClick = new ol.interaction.Select({
			style: styleSelected
		});

		// select interaction working on "click"
		var selectClick = new ol.interaction.Select({
		  condition: ol.events.condition.click
		});

		// select interaction working on "pointermove"
		var selectPointerMove = new ol.interaction.Select({
		  condition: ol.events.condition.pointerMove
		});

		var selectAltClick = new ol.interaction.Select({
		  condition: function(mapBrowserEvent) {
		    return ol.events.condition.click(mapBrowserEvent) &&
		        ol.events.condition.altKeyOnly(mapBrowserEvent);
		  }
		});

		var selectMethod = selectSingleClick;  // ref to currently selected interaction

		var initInteraction = function() {
		  if (selectMethod !== null) {
		    map.removeInteraction(selectMethod);
		  }

		  if (selectMethod !== null) {
		    map.addInteraction(selectMethod);
		    selectMethod.on('select', function(e) {
		      $('#status').html('&nbsp;' + e.target.getFeatures().getLength() +
		          ' selected features (last operation selected ' + e.selected.length +
		          ' and deselected ' + e.deselected.length + ' features)');
		    });
		  }
		};

		initInteraction();

		$('#toggle-one').toggle();
		var treeData;
		var initTree = function() {
			$.ajax({
				url: "getProductTreeData.php",//"productsTreeData.json",
				type: "get",
				cache: false,
				crosDomain: true,
				dataType: "json",
				success: function(data)
				{
					treeData = data;
					renderTree();
					$(".list-group").css("width", $(".tree").width() - 20);
				},
				error: function (responseData, textStatus, errorThrown) {
					console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
					treeData = [];
					renderTree();
				}
			});
		};

		var renderTree = function() {
			var options = {
					data: treeData,
					collapseIcon: "glyphicon glyphicon-folder-open",
					expandIcon: "glyphicon glyphicon-folder-close",
					emptyIcon: "glyphicon glyphicon-file",
					levels: 1,
					enableLinks: false,
					enableTooltips: true,
					enableDownloadButtons: true,
					onNodeExpanded: function (event, data) {
						event.preventDefault();
    	    			if(!data.nodes.length>0){
    					// populate node
    					var	dataNodes = {};
    									
    					if(dataFiltered != {}){
        					var satellite = new Array();
    						var sensors = $("input:checkbox[name='sensor']");
    						$.each(sensors, function(index, sensor){
    							if( $(sensor).is(':checked')){
    								//push sensor id
    				  	   	  		satellite.push($(sensor).val());
    				  	   		    dataFiltered.satellite_id = satellite;
    					  	   	  }
    							});
        					
    						dataFiltered.action = 'getNodes';
    						dataFiltered.product_id = [data.id];
    						dataFiltered.site_id = $('#tree').treeview('getParent', data.nodeId).id;
    						
        					dataNodes = dataFiltered;
    					}else{
    						dataNodes.action = 'getNodes';
        					dataNodes.product_id = [data.id];
        					dataNodes.site_id = $('#tree').treeview('getParent', data.nodeId).id;
        					}
    					$.ajax({
    						  url: "getProductTreeData.php",
    						  type: "get",
    				          dataType : 'json',
    				          data :dataNodes,
    				          success:function(res){
        				                  				        
        				          data.nodes = res.nodes;
        				      	  var nodeId = data.nodeId;
            					  var parentName = $('#tree').treeview('getParent', data.nodeId).text;
            					  var parentIndex = treeData.findIndex(function(obj){return obj.text == parentName;});
            					  treeData[parentIndex].nodes.forEach(function(node,index){
                					  if(node.id == data.id) treeData[parentIndex].nodes[index] = data;
                					});
            					  treeData[parentIndex].state={expanded:true};

            					  //refresh tree
            					  options.data = treeData;
            					  $('#tree').treeview(options);               				   
              				  	
              					  setTimeout(function(){
                  					  $('#tree').treeview('revealNode', [  data.nodeId, { silent: true } ]);
              					  });
              					
        				          }
        					});
    					
    				
    						}

					},
				onNodeCollapsed: function (event, data) {
					event.preventDefault();
					if(data.parentId == undefined){
						var index = treeData.findIndex(function(obj){return obj.text == data.text;});
						treeData[index].state={expanded:false};
					}else{
						 var nodeId = data.nodeId;
   					     var parentName = $('#tree').treeview('getParent', data.nodeId).text;
   					     var parentIndex = treeData.findIndex(function(obj){return obj.text == parentName;});
   					     treeData[parentIndex].nodes.forEach(function(node,index){
       					  if(node.id == data.id) treeData[parentIndex].nodes[index].state = {expanded:false};
       					});
						
						}
				},
				onNodeSelected: function(event, data) {
					var selectedFeatures = vector.getSource();//selectMethod.getFeatures();
					selectedFeatures.clear();
					
					
					//if(productType == "image"){
						var extent;    					
    				
    					if(data['productCoord']) {
    						extent = ol.extent.applyTransform(data['productCoord'], ol.proj.getTransform("EPSG:4326", "EPSG:3857"));
    						
    						var feature = new ol.Feature({
    							geometry: ol.geom.Polygon.fromExtent(extent)
    						});
    						selectedFeatures.addFeature(feature);
    
    					} else {
    						extent = ol.extent.applyTransform([-90,-50,90,50], ol.proj.getTransform("EPSG:4326", "EPSG:3857"));
    					}
    
    					displayProductImage('getProductImage.php?id='+data['productId'],/* data['productImageWidth'], data['productImageHeight'],*/ extent);
    					
    			        vector.getSource().once('change', function(e) {
    			        	  map.getView().fit(extent, {duration: 1000})
    			        	});
					/*}else{
						productType++;
						
						displayProductVector('http://sen2agri-dev:6767/CZ_SEN4CAP_TestSite2/{z}/{x}/{y}.pbf');
						
					    //Fit the map's view to our combined extent
					    vector.getSource().once('change', function(e) {
    			        	  map.getView().fit(e.target.getExtent(), {duration: 1000})
    			        	});
						}*/
					
					//Create polygone for the site
					var format = new ol.format.WKT();
					var feature2 = format.readFeature(data.siteCoord);
					feature2.getGeometry().transform('EPSG:4326', 'EPSG:3857');

					selectedFeatures.addFeature(feature2);
					}
				};
			
			$('#tree').treeview(options);
			
			$('#tree').on('click', function(event, data) {
				var max = 350;
				$(".floatingleaf").each(function(){
					c_width = parseInt($(this).width()) + 100;
					if (c_width > max) { max = c_width; }
				});
				$(".list-group").css("width", max);
			});
		};

		initTree();

	</script>

<?php include "ms_foot.php" ?>
