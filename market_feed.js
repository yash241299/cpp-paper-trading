// market_feed.js
const WebSocket = require("ws");
const Redis = require("ioredis");
const redis = new Redis(); // localhost:6379

// Connect to Binance USDT-M futures trades stream
const ws = new WebSocket("wss://fstream.binance.com/ws/btcusdt@trade");

ws.on("open", () => {
  console.log(
    '🟢 Binance WS connected, publishing ticks → Redis "ticks" channel'
  );
});

ws.on("message", (data) => {
  try {
    const j = JSON.parse(data);
    const tick = {
      event: "TICK",
      symbol: j.s, // "BTCUSDT"
      price: parseFloat(j.p),
      time: j.T, // timestamp in ms
    };
    redis.publish("ticks", JSON.stringify(tick));
  } catch (e) {
    console.error("Feed parse error", e);
  }
});

ws.on("error", (err) => {
  console.error("Binance WS error", err);
});

ws.on("close", () => {
  console.log("⚪ Binance WS closed, exiting");
  process.exit();
});
