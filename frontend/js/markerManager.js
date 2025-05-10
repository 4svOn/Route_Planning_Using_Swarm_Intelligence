import { Feature } from 'ol';
import { Point } from 'ol/geom';
import { Style, Icon } from 'ol/style';
import { toLonLat } from 'ol/proj';

export class MarkerManager {
  constructor(mapManager) {
    this.mapManager = mapManager;
    this.markersList = document.getElementById('markers-list');
    this.styles = {
      customer: this._createStyle('./icons/customer.png', 0.075),
      depot: this._createStyle('./icons/depot.png', 0.125),
      customerSelected: this._createStyle('./icons/customer.png', 0.125),
      depotSelected: this._createStyle('./icons/depot.png', 0.2)
    };
    this.lastUnusedUniqueId = 0;
    this.markersMap = new Map();
    this.addressesMap = new Map();
    this.demandsMap = new Map();
  }

  _createStyle(iconUrl, scale) {
    return new Style({
      image: new Icon({ src: iconUrl, scale }),
      zIndex: 999
    });
  }

  // Функция для форматирования адреса
  _formatAddress(address) {
    const parts = [];
    if (address.house_number) parts.push(address.house_number);
    if (address.road) parts.push(address.road);
    if (address.city) parts.push(address.city);
    // if (address.country) parts.push(address.country);
    return parts.join(', ');
  }

  async getAddressFromCoordinates(coords, uid) {
    // Преобразуем координаты карты в WGS84 (широта/долгота)
    const [lon, lat] = toLonLat(coords);

    // Формируем URL для запроса
    const url = `https://nominatim.openstreetmap.org/reverse?lat=${lat}&lon=${lon}&format=json`;

    try {
      // Выполняем запрос с указанием User-Agent (обязательно!)
      const response = await fetch(url, {
        headers: {
          'User-Agent': 'route-planning-using-swarm-intelligence', // Укажите название вашего приложения
        },
      });

      if (!response.ok) {
        throw new Error('Ошибка запроса');
      }

      const data = await response.json();

      // Форматируем адрес
      if (data.address && data.address !== ' ') {
        this.addressesMap.set(uid, this._formatAddress(data.address));
      } else {
        this.addressesMap.set(uid, 'Адрес не найден');
      }
    } catch (error) {
      console.error('Ошибка:', error);
      this.addressesMap.set(uid, 'Не удалось получить адрес');
    }

    this.updateMarkersList();
  }

  addMarkerToSidePanel(feature, title, listToAdd, isClickable = true) {
    const li = document.createElement('li');
    li.className = 'marker-item';

    let address = this.addressesMap.get(feature.get('uid'));
    if (address === undefined) {
      address = 'Обработка адреса...'
    }

    const isDepot = feature.get('markerType') === 'depot';

    const inputID = `marker-demand-${feature.get('uid')}`;
    // не работает, нужно отдельное хранилище для значений((((
    const value = this.demandsMap.has(feature.get('uid')) ? this.demandsMap.get(feature.get('uid')) : (isDepot ? '100' : '50');
    const inputHolderHTML = `
      <div class="markers-list-number-input-container">
        <span class="markers-list-number-input-label">${isDepot ? 'Capacity' : 'Demand'}:</span>
        <input type="number" id="${inputID}" class="markers-list-number-input" inputmode="numeric" pattern="\d*" placeholder="Input a number" value="${value}" ${isClickable ? '' : 'disabled'}>
      </div}>
      </div>
    `;

    li.innerHTML = `
      <div>
        <div class="list-content-wrapper">
          <div class="list-title-line-wrapper">
            <strong id="list-title-${feature.get('uid')}" ${isDepot ? 'style="font-size: 20px"' : ''}>${title}</strong>
          </div>
          <div class="list-content-address">${address}</div>
          ${inputHolderHTML}
        </div>
      </div>
      ${isClickable ? '<div class="remove-marker">✕</div>' : ''}
    `;

    li.addEventListener('input', (event) => {
      this.demandsMap.set(feature.get('uid'), event.target.value);
    });

    li.addEventListener('click', (event) => {
      const targetElement = event.target.closest('.list-title-line-wrapper');
      if (targetElement) {
        const wasActive = targetElement.classList.contains('active');

        document.querySelectorAll('.list-title-line-wrapper').forEach(element => {
          element.classList.remove('active');
        });

        this.markersMap.forEach(marker => {
          marker.setStyle(this.styles[marker.get('markerType')]);
        });

        if (!wasActive) {
          this.mapManager.centerMap(feature.getGeometry().getCoordinates());

          targetElement.classList.add('active');

          feature.setStyle(this.styles[feature.get("markerType") + 'Selected']);
        }
      }
    });

    if (isClickable) {
      li.querySelector('.remove-marker').addEventListener('click', () => {
        this.removeMarker(feature);
      });
    }

    listToAdd.appendChild(li);
  }

  updateMarkersList() {
    this.markersList.innerHTML = '';

    // Добавляем склад
    const depotFeatures = this.mapManager.getLayer('depot').source.getFeatures();
    depotFeatures.forEach(feature => {
      this.addMarkerToSidePanel(feature, 'Depot', this.markersList);
    });

    // Добавляем клиентов
    const customerFeatures = this.mapManager.getLayer('customers').source.getFeatures();
    customerFeatures.forEach((feature, index) => {
      this.addMarkerToSidePanel(feature, `Customer ${index + 1}`, this.markersList);
    });
  }

  addMarker(coordinates) {
    const type = document.getElementById('toggle-button-customer-depot').checked ? 'depot' : 'customer';

    if (type === 'depot') {
      this.clearMarkers('depot');
    }

    const uid = this.lastUnusedUniqueId++;

    this.getAddressFromCoordinates(coordinates, uid);

    const feature = new Feature({
      geometry: new Point(coordinates),
      markerType: type,
      type: 'marker',
      uid: uid
    });

    feature.setStyle(this.styles[type]);
    this.mapManager.getLayer(type === 'depot' ? 'depot' : 'customers').source.addFeature(feature);

    this.markersMap.set(uid, feature);
    return feature;
  }

  removeMarker(feature) {
    const type = feature.get('markerType');
    const source = this.mapManager.getLayer(type === 'depot' ? 'depot' : 'customers').source;
    source.removeFeature(feature);
    this.markersMap.delete(feature.get('uid'));
    this.updateMarkersList();
  }

  clearMarkers(type) {
    this.markersMap.forEach(marker => {
      if (marker.get('markerType') === type) {
        this.removeMarker(marker);
      }
    });
    this.updateMarkersList();
  }

  reloadMarkersOnMap() {
    this.markersMap.forEach(marker => {
      marker.setStyle(this.styles[marker.get("markerType")]);
    });
  }

  getMarkerFeature(uid) {
    return this.markersMap.get(uid);
  }

  getMarkersData(type) {
    const features = this.mapManager.getLayer(type).source.getFeatures();
    return features.map(feature => {
      const coords = feature.getGeometry().getCoordinates();
      const demand = parseInt(document.getElementById("marker-demand-" + feature.get('uid')).value);
      return {
        coordinates: toLonLat(coords),
        uid: feature.get('uid'),
        demand: demand
      }
    });
  }
}