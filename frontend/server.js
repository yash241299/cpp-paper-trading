// server.js
const redis = require("redis");
const WebSocket = require("ws");

// 1) Subscribe to your Redis channel
const subscriber = redis.createClient();
subscriber.on("error", (err) => console.error("Redis Error", err));
subscriber.subscribe("orders");

// 2) Start a WebSocket server
const wss = new WebSocket.Server({ port: 8080 });
console.log("WebSocket server → ws://localhost:8080");

// 3) Relay Redis messages to all WS clients
subscriber.on("message", (channel, message) => {
  console.log(`→ ${message}`);
  wss.clients.forEach((ws) => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(message);
    }
  });
});
