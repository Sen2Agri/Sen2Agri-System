<?php include 'master.php'; ?>
    <div id="main">
        <div id="main2">
            <div id="main3">
				<table cellpadding="0px" cellspacing="0px">
					<tr>
						<td colspan=3"><h3>Available products</h3></td>
					</tr>
					<tr>
						<td><div id="tree" style="height:550px; width:350px; overflow: scroll; border: 1px solid; font-size: 13px"></div></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>
						<td><div id="map" class="map" style="height:550px; width:665px; border: 1px solid;"></div></td>
					</tr>
				</table>
            </div>
        </div>
    </div><!-- main --><!-- main2 --><!-- main3 -->
	
    <script src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-2.1.4.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.6/d3.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.time.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.stack.min.js"></script>
    <script src="libraries/nvd3-1.1.11/nv.d3.js"></script>
    <script src="libraries/bootstrap-treeview/bootstrap-treeview.min.js"></script>
    <script src="libraries/openlayers/build/ol.js"></script>
    <script src="scripts/config.js"></script>
    <script src="scripts/helpers.js"></script>
    
	<script>
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
	
		var raster = new ol.layer.Tile({
		  source: new ol.source.MapQuest({layer: 'sat'})
		});

		/*var vector = new ol.layer.Vector({
		  source: new ol.source.Vector({
		    url: 'productsMapData.json',
		    format: new ol.format.GeoJSON()
		  })
		});*/
		
		var vector = new ol.layer.Vector({
		  source: new ol.source.Vector(),
		  style: styleSelected
		});
		vector.setZIndex(10);
		
		var view = new ol.View({
		    center: [0, 0],
		    zoom: 2
		});

		var map = new ol.Map({
		  layers: [raster, vector], // [raster, vector]
		  target: 'map',
		  view: view
		});
		
		//map.addLayer(vector);

		// Create an image layer
		var imageLayer;
		
		var displayProductImage = function(imageUrl, imageWidth, imageHeight, extent) {
			if(imageLayer) map.removeLayer(imageLayer);
			
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
					imageSize: [imageWidth, imageHeight],
					projection: map.getView().getProjection(),
					imageExtent: extent
				})
			});
			
			map.addLayer(imageLayer);
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


		var panToLocation = function(extent) {
		  var pan = ol.animation.pan({
		    duration: 1000,
		    source: /** @type {ol.Coordinate} */ (view.getCenter())
		  });
		  map.beforeRender(pan);
		  //view.setCenter(location);
		  //view.setZoom(zoom);
		  view.fit(extent, map.getSize());
		  var zoom = view.getZoom()-3;
		  view.setZoom(zoom);
		};

		initInteraction();





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
				},
				error: function (responseData, textStatus, errorThrown) {
					console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
					treeData = [];
					renderTree();
				}
			});
		};

		var renderTree = function() {
			$('#tree').treeview({
				data: treeData,
				//multiSelect: true,
				//showCheckbox: true,
				collapseIcon: "glyphicon glyphicon-folder-open",
				expandIcon: "glyphicon glyphicon-folder-close",
				emptyIcon: "glyphicon glyphicon-file",
				levels: 1,
				enableLinks: true
				
			});
			
			$('#tree').on('nodeSelected', function(event, data) {
				
				
				var extent;
							
				var selectedFeatures = vector.getSource();//selectMethod.getFeatures();
				selectedFeatures.clear();
				
				if(data['productCoord']) {
					extent = ol.extent.applyTransform(data['productCoord'], ol.proj.getTransform("EPSG:4326", "EPSG:3857"));
					//console.log(data.productCoord);		
					var feature = new ol.Feature({
						geometry: ol.geom.Polygon.fromExtent(extent)
					});
					//selectedFeatures.push(feature);
					selectedFeatures.addFeature(feature);
					
				} else {
					extent = ol.extent.applyTransform([-90,-50,90,50], ol.proj.getTransform("EPSG:4326", "EPSG:3857"));
				}
				
				displayProductImage(data['productImageUrl'], data['productImageWidth'], data['productImageHeight'], extent);
						
				panToLocation(extent);
				
				//Create polygone for the site
				var olCoordArr = new Array();
				//console.log(data);
			    for(var i=0; i<data.siteCoord.length;i++){
					pointCoords = data.siteCoord[i].split(" ");
					olCoordArr.push([Number(pointCoords[0]), Number(pointCoords[1])]);
				}
			   // console.log(olCoordArr);
				polygon = new ol.geom.Polygon([olCoordArr]);
			
				polygon.transform('EPSG:4326', 'EPSG:3857');
				// Create feature with polygon.
				var feature2 = new ol.Feature(polygon);

				selectedFeatures.addFeature(feature2);
				
			});
		};

		initTree();

	
	</script>
   
<?php include 'ms_foot.php'; ?>