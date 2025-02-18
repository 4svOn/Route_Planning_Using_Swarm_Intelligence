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
import axios from 'axios';

// Загрузите .proto файлы
const common_root = await protobuf.load('./proto/common.proto');
const request_root = await protobuf.load('./proto/request.proto');
const response_root = await protobuf.load('./proto/response.proto');


// Получите типы сообщений
const Coordinate_pb = common_root.lookupType('pb.Coordinate');
const Request_pb = request_root.lookupType('pb.Request');
const Response_pb = response_root.lookupType('pb.Response');

const SERVER_URL = 'http://localhost:8080/hello';

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

// Создаем слой для маркеров
const markersSource = new VectorSource();
const markersLayer = new VectorLayer({
    source: markersSource
});
map.addLayer(markersLayer);

// Создаем попап
// const popup = new Overlay({
//   element: document.createElement('div'),
//   positioning: 'bottom-center',
//   stopEvent: false
// });
// map.addOverlay(popup);


// Стиль для маркера (пина)
const pinStyle = new Style({
    image: new Icon({
        src: 'https://openlayers.org/en/latest/examples/data/icon.png', // URL иконки
        scale: 0.8 // Масштаб иконки
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

  // Получаем все маркеры из vectorSource
  const features = markersSource.getFeatures();

  // Собираем координаты маркеров
  const markersData = features.map((feature) => {
    const coordinates = feature.getGeometry().getCoordinates();
    const [longitude, latitude] = toLonLat(coordinates); // Преобразуем координаты, если нужно
    return { longitude, latitude };
  });

  // Создаём запрос на сервер
  const request = Request_pb.create({
    markers: markersData.map((marker) => Coordinate_pb.create(marker))
  });

  try {
    console.log(JSON.stringify(request.toJSON(), null, 2));
    console.log(Request_pb.encode(request).finish());
    // Отправляем GET-запрос на сервер
    const response = await axios.post(
      SERVER_URL,
      Request_pb.encode(request).finish(),
      {
        headers: {
          'Content-Type': 'application/protobuf'
        },
        responseType: "arraybuffer", // Указываем, что ожидаем бинарные данные
      }
    );

    // Выводим ответ сервера на страницу
    responseTextElement.textContent = `Ответ сервера: ${response.data}`;
    const binaryResponse = new Uint8Array(response.data);
    const response_pb = Response_pb.decode(binaryResponse);
    console.log(JSON.stringify(response_pb.toJSON(), null, 2));
    const decodedCoordinates = polyline.decode(response_pb.polyline);
    const routePoints = decodedCoordinates.map(coord => ol.proj.fromLonLat([coord[1], coord[0]]));

    // Создаем линию (маршрут)
    const routeFeature = new ol.Feature({
      geometry: new ol.geom.LineString(routePoints),
    });
    routeSource.addFeature(routeFeature);

    // Опционально: Увеличиваем масштаб карты, чтобы маршрут был виден целиком
    const extent = routeFeature.getGeometry().getExtent();
    map.getView().fit(extent, { padding: [50, 50, 50, 50] });
  } catch (error) {
    // Обрабатываем ошибку
    responseTextElement.textContent = `Ошибка: ${error.message}`;
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

    if (clickedFeature) {
      // Если маркер уже есть, удаляем его
      markersSource.removeFeature(clickedFeature);
    } else {
      // Создаем маркер (пин)
      const pinFeature = new Feature({
          geometry: new Point(coordinates)
      });
      pinFeature.setStyle(pinStyle);

      // Очищаем предыдущие маркеры (если нужно)
      // vectorSource.clear();

      // Добавляем маркер на слой
      markersSource.addFeature(pinFeature);
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

