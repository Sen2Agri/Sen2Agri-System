<?php include "master.php"; ?>
    <div id="main">
        <div id="main2">
            <div id="main3">
				<table cellpadding="0px" cellspacing="0px">
					<tr>
						<td><div id="tree" class="tree"></div></td>
						<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>
						<td><div id="map" class="map"></div></td>
					</tr>
				</table>
            </div>
        </div>
    </div>
	
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
			$('#tree').treeview({
				data: treeData,
				//multiSelect: true,
				//showCheckbox: true,
				collapseIcon: "glyphicon glyphicon-folder-open",
				expandIcon: "glyphicon glyphicon-folder-close",
				emptyIcon: "glyphicon glyphicon-file",
				levels: 1,
				enableLinks: false,
				enableTooltips: true,
				enableDownloadButtons: true
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
				var format = new ol.format.WKT();
				var feature2 = format.readFeature(data.siteCoord);
				feature2.getGeometry().transform('EPSG:4326', 'EPSG:3857');
				
				selectedFeatures.addFeature(feature2);
			});
			
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
