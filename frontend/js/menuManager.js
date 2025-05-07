// export class MenuManager {
//   constructor(mapManager) {
//     this.mapManager = mapManager;
//     this.popup = document.getElementById('popup');
//     this.popupValue = document.getElementById('popup-total-distance');
//     this.errorMessage = document.getElementById('error-message');
//     this.errorText = document.getElementById('error-text');\

//     this._initEventListeners();

//   }

//   init() {
//     // Инициализация меню
//   }

//   _initEventListeners() {
//     document.getElementById('sendRequestBtn').addEventListener('click', () => this.handleRequest());
//     document.getElementById('clearRoutesBtn').addEventListener('click', () => this.clearAll());
//     document.getElementById('toggle-button-customer-depot').addEventListener('change', (e) => {
//       this.markerType = e.target.checked ? 'depot' : 'customer';
//     });
//   }
// }