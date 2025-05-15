# C++ Paper Trading System with OCO Orders & Live Binance Perpetual Futures

A lightweight, stateless C++ paper-trading engine that supports OCO orders, real-time Redis Pub/Sub communication (both order events and live BTCUSDT perpetual-futures ticks), an HTTP/REST API, and a React front-end UI.

## Project Structure

cpp-paper-trading/
├── CMakeLists.txt # CMake build for C++ backend & tests
├── market_feed.js # Node.js bridge: Binance fstream → Redis “ticks”
├── README.md # This file
├── include/ # Public headers
│ ├── CommunicationLayer.hpp
│ ├── OrderEngine.hpp
│ ├── ApiServer.hpp
│ └── json.hpp # nlohmann::json single-header
├── src/ # C++ implementation
│ ├── CommunicationLayer.cpp
│ ├── OrderEngine.cpp
│ ├── ApiServer.cpp
│ └── main.cpp # wires up Redis subscriber + HTTP API
├── tests/ # C++ unit tests (GoogleTest)
│ └── OrderEngineTests.cpp
└── frontend/
├── server.js # Node.js WebSocket proxy: Redis → browser WS
└── app/ # React application
├── package.json
├── public/
│ ├── index.html
│ └── ...
└── src/
├── index.js
├── App.js
└── components/
├── OrderForm.jsx
├── OcoForm.jsx
├── EventList.jsx
└── PriceChart.jsx # live tick chart

## Prerequisites

- **macOS** with Xcode command-line tools
- **Homebrew** packages:

  ```bash
  brew install cmake redis hiredis node boost nlohmann-json

  ```

- **OtherOS** c++ 17 must be installed

1. Run Redis
   brew services start redis
   redis-cli ping

2. Start the Binance “fstream” → Redis Bridge
   cd cpp-paper-trading
   npm install ws ioredis
   node market_feed.js

# This script connects to Binance’s USDT‐margined perpetual futures WebSocket and publishes each trade as

# { "event":"TICK", "symbol":"BTCUSDT", "price":<number>, "time":<ms> }

3. Build & Run the C++ Backend
   cd cpp-paper-trading
   rm -rf build && mkdir build && cd build
   cmake ..
   make
   ./trading_engine

# Seeds a demo OCO group on BTCUSDT

# Spawns a Redis subscriber thread on “ticks” → calls OrderEngine::onMarketTick(...)

# Launches HTTP API on port 8081 (endpoints /order and /oco)

# Publishes order events (OPEN/FILLED/CANCELED) to Redis channel “orders”

4. Start the WebSocket Proxy for the React App
   cd frontend
   npm install redis ws
   node server.js

# This script subscribes to Redis channels “orders” and “ticks” and forwards every JSON message to any connected browser clients via WebSocket on ws://localhost:8080.

5. Launch the React Frontend
   cd frontend/app
   npm install
   npm start

# Open your browser at http://localhost:3000. You’ll see:

# - Live Price Chart (BTCUSDT perpetual futures ticks)

# - Order Forms (single LIMIT/MARKET and OCO)

# - Event List showing OPEN, FILLED, CANCELED badges and order/type

# - Place orders in the form—events will appear in real time alongside the live chart.

6. Run C++ Unit Tests
   cd cpp-paper-trading/build
   ctest --output-on-failure

./OrderEngineTests

# Notes & Further Reading

# - /src/main.cpp now uses a Redis subscriber instead of the old simulator

# - You can remove MarketDataSimulator.\* once you’re happy with live data

# - Extend tests in /tests/ to cover OCO, MARKET/LIMIT logic, cancels, etc.

# - Future enhancements: Binance REST fallback, Docker Compose setup, persistent order store
