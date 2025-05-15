#include "ApiServer.hpp"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

ApiServer::ApiServer(OrderEngine& engine, int port)
  : engine_(engine)
{
  svr_.set_default_headers({
    {"Access-Control-Allow-Origin",  "*"},
    {"Access-Control-Allow-Methods", "POST, OPTIONS"},
    {"Access-Control-Allow-Headers", "Content-Type"},
  });

  setupRoutes();

  std::cout << "API Server listening on port " << port << "\n";
  svr_.listen("0.0.0.0", port);
}

ApiServer::~ApiServer() {
  svr_.stop();
}

void ApiServer::setupRoutes() {
  svr_.Options("/order", [](auto& req, auto& res){
    res.status = 200;
  });
  svr_.Options("/oco", [](auto& req, auto& res){
    res.status = 200;
  });

  // POST /order
  svr_.Post("/order", [&](const httplib::Request& req, httplib::Response& res){
    try {
      auto j = json::parse(req.body);
      Side side = (j.at("side") == "BUY") ? Side::BUY : Side::SELL;
      Type type = (j.at("type") == "MARKET") ? Type::MARKET : Type::LIMIT;

      Order o{
        j.at("id").get<std::string>(),
        j.at("symbol").get<std::string>(),
        side,
        j.at("price").get<double>(),
        j.at("quantity").get<double>(),
        type,
        Status::OPEN
      };
      engine_.placeOrder(o);

      res.status = 200;
      res.set_content(R"({"result":"ok"})", "application/json");
    } catch (const std::exception& e) {
      res.status = 400;
      res.set_content(
        std::string("{\"error\":\"") + e.what() + "\"}",
        "application/json"
      );
    }
  });

  // POST /oco
  svr_.Post("/oco", [&](const httplib::Request& req, httplib::Response& res){
    try {
      auto j = json::parse(req.body);
      std::string ocoId = j.at("ocoId").get<std::string>();
      std::vector<Order> orders;

      for (auto& item : j.at("orders")) {
        Side side = (item.at("side") == "BUY") ? Side::BUY : Side::SELL;
        Type type = (item.at("type") == "MARKET") ? Type::MARKET : Type::LIMIT;

        orders.push_back(Order{
          item.at("id").get<std::string>(),
          item.at("symbol").get<std::string>(),
          side,
          item.at("price").get<double>(),
          item.at("quantity").get<double>(),
          type,
          Status::OPEN
        });
      }

      engine_.placeOCOOrder(ocoId, orders);

      res.status = 200;
      res.set_content(R"({"result":"ok"})", "application/json");
    } catch (const std::exception& e) {
      res.status = 400;
      res.set_content(
        std::string("{\"error\":\"") + e.what() + "\"}",
        "application/json"
      );
    }
  });
}
