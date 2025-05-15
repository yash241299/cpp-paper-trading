#ifndef API_SERVER_HPP
#define API_SERVER_HPP

#include "httplib.h"
#include "OrderEngine.hpp"
#include <string>

class ApiServer {
public:
  ApiServer(OrderEngine& engine, int port = 8081);
  ~ApiServer();
  void listen();

private:
  OrderEngine& engine_;
  httplib::Server svr_;
  void setupRoutes();
};

#endif
