import 'ol/ol.css';
import { Map, View } from "ol";
import OSM from "ol/source/OSM";
import TileLayer from "ol/layer/Tile";
import { fromLonLat, toLonLat } from 'ol/proj';
import { Feature } from 'ol';
import { Point } from 'ol/geom';
import { Vector as VectorLayer } from 'ol/layer';
import { Vector as VectorSource } from 'ol/source';
import { Style, Icon } from 'ol/style';
import polyline from "@mapbox/polyline";


// import { MarkersList, Marker } from './markers.proto';

// const { Request } = require('../proto/js/request_pb.js');
// const { Marker, MarkersList } = require('./proto/markers_pb.js');
// import { Marker, MarkersList } from './proto/markers_pb.js';
// import protobuf from 'protobufjs';

// Загрузите protobuf-схему
// const root = await protobuf.load('../proto/markers.proto');

// Получите типы сообщений
// const MarkersList = root.lookupType('MarkersList');
// const Marker = root.lookupType('Marker');

import protobuf from 'protobufjs';
// import axios from 'axios';
// import superagent from 'superagent';
// import ky from 'ky';


// Загрузите .proto файлы
const common_root = await protobuf.load('./proto/common.proto');
const request_root = await protobuf.load('./proto/request.proto');
const response_root = await protobuf.load('./proto/response.proto');


// Получите типы сообщений
const Coordinate_pb = common_root.lookupType('pb.Coordinate');
const Customer_pb = request_root.lookupType('pb.Customer');
const Request_pb = request_root.lookupType('pb.Request');
const Route_pb = response_root.lookupType('pb.Route');
const Response_pb = response_root.lookupType('pb.Response');

const SERVER_URL = 'http://localhost:8080/hello';

const routesColors = [
  'rgba(255, 0, 0, 0.8)',   // Красный
  'rgba(0, 255, 0, 0.8)',   // Зелёный
  'rgba(0, 0, 255, 0.8)',    // Синий
  'rgba(255, 255, 0, 0.8)',  // Жёлтый
  'rgba(255, 0, 255, 0.8)',  // Пурпурный
  'rgba(0, 255, 255, 0.8)',  // Голубой
];

// Создаём экземпляр карты
const map = new Map({
  // HTML-элемент, в который будет инициализирована карта
  target: document.getElementById("map"),
  // Список слоёв на карте
  layers: [
    // Создадим тайловый слой. Источником тайлов будет OpenStreetMap
    new TileLayer({ source: new OSM() }),
  ],
  // Параметры отображения карты по умолчанию: координата центра и зум
  view: new View({
    center: [4190701.0645526173, 7511438.408408914],
    zoom: 10,
  }),
});

// Создаем слой для пинов покупателей
const markersCustomersSource = new VectorSource();
const markersCustomersLayer = new VectorLayer({
    source: markersCustomersSource
});
map.addLayer(markersCustomersLayer);

// Создаем слой для пинов склада
const markersDepotSource = new VectorSource();
const markersDepotLayer = new VectorLayer({
    source: markersDepotSource
});
map.addLayer(markersDepotLayer);

// Создаем попап
// const popup = new Overlay({
//   element: document.createElement('div'),
//   positioning: 'bottom-center',
//   stopEvent: false
// });
// map.addOverlay(popup);


// Стиль для пина покупателя
const pinStyleCustomer = new Style({
    image: new Icon({
        src: 'https://openlayers.org/en/latest/examples/data/icon.png', // URL иконки
        // src: 'https://clipground.com/images/google-map-marker-png-4.jpg', // URL иконки
        scale: 0.8 // Масштаб иконки
    })
});

// Стиль для пина покупателя
const pinStyleDepot = new Style({
  image: new Icon({
      src: 'https://image.pngaaa.com/106/1259106-middle.png', // URL иконки
      scale: 0.02 // Масштаб иконки
  })
});

// Создаем слой для маршрута
const routeSource = new VectorSource();
const routeLayer = new ol.layer.Vector({
  source: routeSource,
  style: new ol.style.Style({
    stroke: new ol.style.Stroke({
      color: 'blue', // Цвет линии
      width: 4,      // Толщина линии
    }),
  }),
});

// Добавляем слой маршрута на карту
map.addLayer(routeLayer);

// Обработчик клика по кнопке
document.getElementById('sendRequestBtn').addEventListener('click', async () => {
  const responseTextElement = document.getElementById('responseText');

  // Получаем все маркеры клиентов
  const featuresCustomers = markersCustomersSource.getFeatures();
  console.log(featuresCustomers);

  // Получаем все маркеры склада
  const featuresDepot = markersDepotSource.getFeatures();
  console.log(featuresDepot);

  if (featuresCustomers.length === 0) {
    showError('Не выбраны точки клиентов');
    return;
  }

  if (featuresDepot.length === 0 || featuresDepot.length > 1) {
    showError('Не выбрана точка склада или более одной точки склада');
    return;
  }

  const getLatLonFromFeature = (feature) => {
    const coordinates = feature.getGeometry().getCoordinates();
    const [longitude, latitude] = toLonLat(coordinates); // Преобразуем координаты, если нужно
    return { longitude, latitude };
  };

  // Собираем координаты маркеров клиентов
  const markersDataCustomers = featuresCustomers.map((featureCustomer) => {
    return getLatLonFromFeature(featureCustomer);
  });
  console.log("markersDataCustomers:", markersDataCustomers); // Логируем данные маркеров

  // Собираем координаты маркеров клиентов
  const markersDataDepot = featuresDepot.map((featureDepot) => {
    return getLatLonFromFeature(featureDepot);
  });
  console.log("markersDataDepot:", markersDataDepot); // Логируем данные маркеров


  // Создаём запрос на сервер
  const request = Request_pb.create({
    depot: Customer_pb.create({
      coordinate: Coordinate_pb.create(markersDataDepot[0]),
      demand: 100
    }),
    customers: markersDataCustomers.map((marker) => Customer_pb.create({
      coordinate: Coordinate_pb.create(marker),
      demand: 50,
    })),
    timestamp: Date.now()
  });

  const requestBody = Request_pb.encode(request).finish();

  try {
    console.log(JSON.stringify(request.toJSON(), null, 2));
    console.log(requestBody);
    // Отправляем GET-запрос на сервер
    const url = SERVER_URL + "?timestamp=" + Date.now();
    const response = await fetch(url, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/octet-stream',
        'Cache-Control': 'no-cache',
        'Pragma': 'no-cache',
        'Expires': '0',
      },
      body: requestBody, // Бинарные данные
    });
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    const arrayBuffer = await response.arrayBuffer(); // Получаем ArrayBuffer
    // Выводим ответ сервера на страницу
    // responseTextElement.textContent = `Ответ сервера: ${arrayBuffer}`;
    const binaryResponse = new Uint8Array(arrayBuffer);
    const response_pb = Response_pb.decode(binaryResponse);
    console.log(JSON.stringify(response_pb.toJSON(), null, 2));

    routeSource.clear();
    for (let i = 0; i < response_pb.routes.length; i++) {
      const route = response_pb.routes[i];
      const decodedCoordinates = polyline.decode(route.polyline);
      const routePoints = decodedCoordinates.map(coord => ol.proj.fromLonLat([coord[1], coord[0]]));

      // Создаем линию (маршрут)
      const routeFeature = new ol.Feature({
        geometry: new ol.geom.LineString(routePoints),
      });

      // Выбираем цвет из массива
      const color = routesColors[i % routesColors.length]; // Используем остаток от деления для циклического выбора цвета
      // Создаем стиль для маршрута
      const routeStyle = new ol.style.Style({
        stroke: new ol.style.Stroke({
          color: color, // Цвет линии
          width: 4,     // Толщина линии
          // opacity: 80,
        }),
      });
      // Применяем стиль к маршруту
      routeFeature.setStyle(routeStyle);

      routeSource.addFeature(routeFeature);

      // Добавляем стрелки направления
      // addDirectionArrows(routeFeature.getGeometry(), routeSource, color);
    }

    // Опционально: Увеличиваем масштаб карты, чтобы маршрут был виден целиком
    // const extent = routeFeature.getGeometry().getExtent();
    // map.getView().fit(extent, { padding: [50, 50, 50, 50] });
  } catch (error) {
    // Обрабатываем ошибку
    console.log(`Ошибка: ${error.message}`);
  }
});

// Обработчик клика на карте
map.on('click', (event) => {
    // Получаем координаты клика
    const coordinates = event.coordinate;
    // Проверяем, есть ли маркер в точке клика
    const clickedFeature = map.forEachFeatureAtPixel(event.pixel, (feature) => {
        return feature;
    });

    const toggleButton = document.getElementById('toggle-button-customer-depot');

    if (clickedFeature) {
      markersCustomersSource.removeFeature(clickedFeature);
      markersDepotSource.removeFeature(clickedFeature);
      return;
    }

    if (!toggleButton.checked) {
      // покупатель
      // Создаем маркер (пин)
      const pinFeature = new Feature({
          geometry: new Point(coordinates)
      });
      pinFeature.setStyle(pinStyleCustomer);

      // Очищаем предыдущие маркеры (если нужно)
      // vectorSource.clear();

      // Добавляем маркер на слой
      markersCustomersSource.addFeature(pinFeature);
    } else {
      // склад
      // Создаем маркер (пин)
      const pinFeature = new Feature({
          geometry: new Point(coordinates)
      });
      pinFeature.setStyle(pinStyleDepot);

      // Очищаем предыдущие маркеры
      markersDepotSource.clear();

      // Добавляем маркер на слой
      markersDepotSource.addFeature(pinFeature);
    }

  // Выводим координаты в консоль (опционально)
  console.log('Clicked coordinates:', coordinates);
  console.log('Clicked coordinates:', toLonLat(coordinates));
});

// Изменяем курсор при наведении на маркер
map.on('pointermove', (event) => {
  const hasFeature = map.hasFeatureAtPixel(event.pixel);
  map.getTargetElement().style.cursor = hasFeature ? 'pointer' : '';

});

function showError(message) {
  const errorMessage = document.getElementById('error-message');
  const errorText = document.getElementById('error-text');
  errorText.textContent = message;
  errorMessage.classList.add('show');

  setTimeout(() => {
    hideError();
  }, 3000);
}

function addDirectionArrows(lineString, source, color, arrowDistance = 10000) {
  const totalLength = lineString.getLength(); // Общая длина маршрута

  // Размещаем стрелки вдоль маршрута
  for (let dist = 0; dist < totalLength; dist += arrowDistance) {
    // Получаем точку на маршруте на заданном расстоянии
    const point = lineString.getCoordinateAt(dist / totalLength);

    // Находим ближайший отрезок маршрута
    const segment = findSegmentAtDistance(lineString, dist);
    if (!segment) continue;

    // Вычисляем угол наклона отрезка
    const dx = segment[1][0] - segment[0][0];
    const dy = segment[1][1] - segment[0][1];
    const angle = Math.atan2(dy, dx);

    // Создаем точку для стрелки
    const arrowFeature = new ol.Feature({
      geometry: new ol.geom.Point(point),
    });

    // Создаем стиль для стрелки
    const arrowStyle = new ol.style.Style({
      image: new ol.style.Icon({
        src: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="' + color + '"><path d="M12 2L2 22h20z"/></svg>',
        scale: 0.5,
        rotation: angle, // Поворачиваем стрелку в направлении маршрута
      }),
    });

    // Применяем стиль к стрелке
    arrowFeature.setStyle(arrowStyle);

    // Добавляем стрелку на карту
    source.addFeature(arrowFeature);
  }
}

// Функция для поиска отрезка маршрута на заданном расстоянии
function findSegmentAtDistance(lineString, distance) {
  const coordinates = lineString.getCoordinates();
  let accumulatedDistance = 0;

  for (let i = 0; i < coordinates.length - 1; i++) {
    const start = coordinates[i];
    const end = coordinates[i + 1];

    // Вычисляем длину текущего отрезка
    const dx = end[0] - start[0];
    const dy = end[1] - start[1];
    const segmentLength = Math.sqrt(dx * dx + dy * dy);

    // Если расстояние попадает на текущий отрезок
    if (accumulatedDistance + segmentLength < distance) {
      return [start, end]; // Возвращаем отрезок
    }

    // Увеличиваем накопленное расстояние
    accumulatedDistance += segmentLength;
  }

  return null; // Если отрезок не найден
}

// Можем изменить значения любых переданных параметров
// map.setTarget(document.getElementById("#another-place-for-map"));
// map.getView().setZoom(5);
// map.getView().setCenter(5);

// Добавляем обработчик клика по карте
// map.on("click", function (event) {
//   // Кидаем в консоль координаты
//   console.log(event.coordinate);
//   // Часто бывает полезно знать текущий зум
//   console.log(map.getView().getZoom());
//  });

// Конвертация координат из EPSG:4326 в EPSG:3857
// fromLonLat([37.64570817463565, 55.76329720561773]); // Будет [4190701.0645526173, 7511438.408408914]
// // И обратно
// toLonLat([4190701.0645526173, 7511438.408408914]); // [37.64570817463565, 55.76329720561773]

