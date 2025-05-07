export class ApiClient {
    constructor(protoConfig, mapManager, markerManager, routeManager) {
      this.proto = protoConfig;
      this.SERVER_URL = 'http://localhost:8081/hello';
      this.mapManager = mapManager;
      this.markerManager = markerManager;
      this.routeManager = routeManager;
      this.betterSolution = this.proto.Algorithm.values.UNSPECIFIED;
    }

    _bufferToHex(buffer) {
      return Array.from(new Uint8Array(buffer))
        .map(b => b.toString(16).padStart(2, '0'))
        .join(' ');
    }

    async sendRequest() {
      const customers = this.markerManager.getMarkersData('customers');
      const depots = this.markerManager.getMarkersData('depot');
      this._validateRequest(customers, depots);
      const depot = depots[0];

      const request = this.proto.Request.create({
        depot: this._createCustomerProto(depot),
        customers: customers.map(c => this._createCustomerProto(c)),
        timestamp: Date.now(),
        algorithm: this.proto.Algorithm.values.BOTH,
      });

      const requestBuffer = this.proto.Request.encode(request).finish();

      try {
        const response = await fetch(this.SERVER_URL, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/octet-stream',
            'Cache-Control': 'no-cache',
            'Pragma': 'no-cache',
            'Expires': '0',
          },
          body: requestBuffer
        });

        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }

        const arrayBuffer = await response.arrayBuffer();
        // console.log('Raw response buffer:', arrayBuffer);

        // Добавим проверку пустого ответа
        if (!arrayBuffer || arrayBuffer.byteLength === 0) {
          throw new Error('Empty response from server');
        }

        const binaryResponse = new Uint8Array(arrayBuffer);
        // console.log('Response Uint8Array:', binaryResponse);

        const decodedProto = this.proto.Response.decode(binaryResponse);
        this.ACOSolution = decodedProto.ACOSolution;
        this.PSOSolution = decodedProto.PSOSolution;
        this.betterSolution = decodedProto.betterAlgorithm;
      } catch (error) {
        console.error('Failed to decode response:', error);
        throw new Error(`Failed to process server response: ${error.message}`);
      }
    }

    getSolution(algorithm) {
      if (algorithm === this.proto.Algorithm.values.ACO) {
        return this.ACOSolution;
      } else if (algorithm === this.proto.Algorithm.values.PSO) {
        return this.PSOSolution;
      }
    }

    getBetterSolution() {
      if (!this.betterSolution || this.betterSolution === this.proto.Algorithm.values.UNSPECIFIED) {
        return null;
      }
      if (this.betterSolution == this.proto.Algorithm.values.ACO) {
        return this.ACOSolution;
      } else {
        return this.PSOSolution;
      }
    }

    _validateRequest(customers, depot) {
      if (customers.length === 0) {
        throw new Error('No customer points selected');
      }

      if (depot.length === 0 || depot.length > 1) {
        throw new Error('No depot point selected or more than one point selected');
      }

      const vehicleCapacity = depot[0].demand;

      for (const customer of customers) {
        if (customer.demand > vehicleCapacity) {
          throw new Error('Customer demand is greater than vehicle capacity');
        }
      }
    }

    _createCustomerProto(marker) {
      return this.proto.Customer.create({
        coordinate: this.proto.Coordinate.create({
          longitude: marker.coordinates[0],  // явно передаём longitude
          latitude: marker.coordinates[1],    // явно передаём latitude
        }),
        demand: marker.demand,
        uid: marker.uid
      });
    }
  }