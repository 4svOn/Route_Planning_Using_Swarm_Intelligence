import { Style, Stroke } from 'ol/style';
import { fromLonLat } from 'ol/proj';
import { Feature } from 'ol';
import { LineString } from 'ol/geom';
import polyline from "@mapbox/polyline";

export class RouteManager {
  constructor(mapManager, markerManager) {
    this.mapManager = mapManager;
    this.markerManager = markerManager;
    this.colors = [
      'rgba(255, 0, 0, 0.8)',
      'rgba(0, 255, 0, 0.8)',
      'rgba(0, 0, 255, 0.8)',
      'rgba(255, 255, 0, 0.8)',
      'rgba(255, 0, 255, 0.8)',
      'rgba(0, 255, 255, 0.8)',
    ];
    this.routesMapFeatures = new Map();
    this.routesMapUIDs = new Map();
    this.listTitle = document.getElementById('route-markers-list-title');
    this.listContainer = document.getElementById('route-markers-list-container');
  }

  drawRoute(polylineStr, index, uids) {
    const decoded = polyline.decode(polylineStr);
    const points = decoded.map(coord => fromLonLat([coord[1], coord[0]]));
    const style = this._createRouteStyle(index);

    const feature = new Feature({
      geometry: new LineString(points),
      routeIndex: index,
      type: 'route',
      style: style,
      isHighlighted: false
    });

    feature.setStyle(style);
    this.routesMapFeatures.set(index, feature);
    this.routesMapUIDs.set(index, uids);

    this.mapManager.getLayer('routes').source.addFeature(feature);
  }

  unhighlightAllRoutes() {
    this.mapManager.getLayer('routes').source.getFeatures().forEach((f) => {
      let style = f.get('style');
      style.setZIndex(997); // Устанавливаем высокий z-index
      style.getStroke().setWidth(4);
      f.setStyle(style);
      f.set('isHighlighted', false);
    });

    this.markerManager.reloadMarkersOnMap();
    this.listTitle.classList.add('hidden-element');
    this.listContainer.classList.add('hidden-element');
  }

  clearRoutes() {
    this.unhighlightAllRoutes();
    this.mapManager.getLayer('routes').source.clear();
    this.routesMapFeatures.clear();
    this.routesMapUIDs.clear();
  }

  _createRouteStyle(index) {
    return new Style({
      stroke: new Stroke({
        color: this.colors[index % this.colors.length],
        width: 4
      })
    });
  }

  highlightRoute(feature) {
    // this.unhighlightAllRoutes();
    const style = feature.get('style');
    style.setZIndex(998); // Устанавливаем высокий z-index
    style.getStroke().setWidth(7);
    feature.setStyle(style);

    feature.set('isHighlighted', true);

    this.listContainer.innerHTML = '';

    const uids = this.routesMapUIDs.get(feature.get('routeIndex'));
    uids.forEach((uid, index) => {
      if (index == 0 || index == uids.length - 1) {
        return;
      }
      this.markerManager.addMarkerToSidePanel(this.markerManager.getMarkerFeature(uid), document.getElementById('list-title-' + uid).textContent, this.listContainer, false);
    });

    this.listTitle.classList.remove('hidden-element');
    this.listContainer.classList.remove('hidden-element');
  }
}