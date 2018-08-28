var scaleLineControl = new ol.control.ScaleLine();

var map = new ol.Map({
  controls: ol.control.defaults({
    attributionOptions: {
      collapsible: false
    }
  }).extend([
    scaleLineControl
  ]),
  layers: [
    new ol.layer.Tile({
      source: new ol.source.OSM()
    })
  ],
  target: 'map',
  view: new ol.View({
    center: [0, 0],
    zoom: 2
  })
});


var unitsSelect = document.getElementById('units');
function onChange() {
  scaleLineControl.setUnits(unitsSelect.value);
}
unitsSelect.addEventListener('change', onChange);
onChange();
