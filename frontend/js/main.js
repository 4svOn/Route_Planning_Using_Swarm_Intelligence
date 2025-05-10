import { MapManager } from './mapManager.js';
import { MarkerManager } from './markerManager.js';
import { RouteManager } from './routeManager.js';
import { ApiClient } from './apiClient.js';

import protobuf from 'protobufjs';

export class App {
  constructor(protoConfig) {
    this.mapManager = new MapManager('map');
    this.markerManager = new MarkerManager(this.mapManager);
    this.routeManager = new RouteManager(this.mapManager, this.markerManager);
    this.apiClient = new ApiClient(protoConfig, this.mapManager, this.markerManager, this.routeManager);
    this.popup = document.getElementById('popup');
    this.popupValue = document.getElementById('popup-total-distance');
    this.errorMessage = document.getElementById('error-message');
    this.errorText = document.getElementById('error-text');

    this.algorithm_toggle = document.getElementById('toggle-switch-aco-pso');

    this._initEventListeners();
    this._setupMapInteractions();

  }

  _initEventListeners() {
    document.getElementById('sendRequestBtn').addEventListener('click', () => this.handleRequest());
    document.getElementById('clearRoutesBtn').addEventListener('click', () => this.clearAll());
    document.getElementById('toggle-button-customer-depot').addEventListener('change', (e) => {
      this.markerType = e.target.checked ? 'depot' : 'customer';
    });
    document.getElementById('toggle-button-aco-pso').addEventListener('change', (e) => {
      this.algorithm = e.target.checked ? this.apiClient.protoConfig.Algorithm.values.PSO : this.apiClient.protoConfig.Algorithm.values.ACO;
      this.displaySolution(this.apiClient.getSolution(this.algorithm));
    });
  }

  _setupMapInteractions() {
    this.mapManager.map.on('click', (e) => this.handleMapClick(e));
    this.mapManager.map.on('pointermove', (e) => this.handlePointerMove(e));
  }

  handleMapClick(event) {
    const clickedFeature = this.mapManager.map.forEachFeatureAtPixel(
      event.pixel,
      (feature) => feature
    );

    if (clickedFeature) {
      if (clickedFeature.get('type') === 'route') {
        if (clickedFeature.get('isHighlighted')) {
          console.log(clickedFeature);
          this.routeManager.unhighlightAllRoutes();
          return;
        }
        this.routeManager.unhighlightAllRoutes();
        this.routeManager.highlightRoute(clickedFeature);
        return;
      }
      this.clearAll();
      this.markerManager.removeMarker(clickedFeature);
      return;
    }

    this.routeManager.unhighlightAllRoutes();
    this.markerManager.addMarker(event.coordinate);
  }

  handlePointerMove(event) {
    const hasFeature = this.mapManager.map.hasFeatureAtPixel(event.pixel);
    this.mapManager.map.getTargetElement().style.cursor = hasFeature ? 'pointer' : '';
  }

  async handleRequest() {
    try {
      await this.apiClient.sendRequest();
      const betterSolution = this.apiClient.getBetterSolution();
      if (!betterSolution) {
        this.showError('No solution found');
        return;
      }
      this.displaySolution(betterSolution);
    } catch (error) {
      this.showError(error.message);
      console.error('Request failed:', error);
    }
  }

  displaySolution(solution) {
    this.routeManager.clearRoutes();

    this.algorithm_toggle.classList.remove('hidden-element');
    document.getElementById('toggle-button-aco-pso').checked = solution.algorithm === this.apiClient.protoConfig.Algorithm.values.PSO;

    solution.routes.forEach((route, index) => {
      this.routeManager.drawRoute(route.polyline, index, route.uids);
    });

    const totalDistance = parseInt(solution.totalDistance);
    const algoText = solution.algorithm === this.apiClient.protoConfig.Algorithm.values.ACO ? "ACO" : "PSO";

    this.popupValue.textContent = `Total distance: ${totalDistance / 1000} km\nSolved by ${algoText}`;
    this.popup.classList.add('show');
  }

  clearAll() {
    this.algorithm_toggle.classList.add('hidden-element');
    this.routeManager.clearRoutes();
    this.popup.classList.remove('show');
  }

  showError(message) {
    this.errorText.textContent = message;
    this.errorMessage.classList.add('show');
    setTimeout(() => this.hideError(), 3000);
  }

  hideError() {
    this.errorMessage.classList.remove('show');
  }
}

// Инициализация приложения после загрузки protobuf
async function initApp() {
  const common_root = await protobuf.load('./proto/common.proto');
  const request_root = await protobuf.load('./proto/request.proto');
  const response_root = await protobuf.load('./proto/response.proto');

  const protoConfig = {
    Coordinate: common_root.lookupType('pb.Coordinate'),
    Algorithm: common_root.lookupEnum('pb.Algorithm'),
    Customer: request_root.lookupType('pb.Customer'),
    Request: request_root.lookupType('pb.Request'),
    Route: response_root.lookupType('pb.Route'),
    Response: response_root.lookupType('pb.Response'),
    Algorithm_values: common_root.lookupEnum('pb.Algorithm').values
  };

  new App(protoConfig);
}

initApp().catch(console.error);