import { Map, View } from "ol";
import OSM from "ol/source/OSM";
import TileLayer from "ol/layer/Tile";
import { Vector as VectorLayer } from 'ol/layer';
import { Vector as VectorSource } from 'ol/source';

export class MapManager {
  constructor(targetId) {
    this.map = new Map({
      target: document.getElementById(targetId),
      layers: [
        new TileLayer({ source: new OSM() }),
      ],
      view: new View({
        center: [4190701.0645526173, 7511438.408408914],
        zoom: 10,
      }),
    });

    this.layers = {
      routes: this._createLayer('routes'),
      customers: this._createLayer('customers'),
      depot: this._createLayer('depot'),
    };
  }

  _createLayer(name) {
    const source = new VectorSource();
    const layer = new VectorLayer({ source });
    this.map.addLayer(layer);
    return { source, layer };
  }

  getLayer(name) {
    return this.layers[name];
  }

  centerMap(center) {
    this.map.getView().setCenter(center);
    this.map.getView().setZoom(12);
  }
}