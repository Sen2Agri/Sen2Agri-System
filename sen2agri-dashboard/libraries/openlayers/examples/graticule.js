var map = new ol.Map({
  layers: [
    new ol.layer.Tile({
      source: new ol.source.OSM({
        wrapX: false
      })
    })
  ],
  target: 'map',
  view: new ol.View({
    center: ol.proj.fromLonLat([4.8, 47.75]),
    extent: ol.proj.get('EPSG:3857').getExtent(),
    zoom: 5
  })
});

// Create the graticule component
var graticule = new ol.Graticule({
  // the style to use for the lines, optional.
  strokeStyle: new ol.style.Stroke({
    color: 'rgba(255,120,0,0.9)',
    width: 2,
    lineDash: [0.5, 4]
  }),
  showLabels: true
});

graticule.setMap(map);
