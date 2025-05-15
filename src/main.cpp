// #include "OrderEngine.hpp"
// #include "CommunicationLayer.hpp"
// #include "MarketDataSimulator.hpp"
// #include "ApiServer.hpp"
// #include <thread>
// #include <vector>
// #include <iostream>

// int main() {
//     // 1) Setup communication & engine
//     CommunicationLayer comm;
//     OrderEngine       engine(comm);

//     // 2) Seed with an example OCO group (using LIMIT orders)
//     Order buy  {"1", "BTCUSD", Side::BUY,  50000.0, 1.0, Type::LIMIT, Status::OPEN};
//     Order sell {"2", "BTCUSD", Side::SELL, 51000.0, 1.0, Type::LIMIT, Status::OPEN};
//     engine.placeOCOOrder("oco-1", {buy, sell});
//     std::cout << "Placed demo OCO orders\n";

//     // 3) Start market-data simulation in background
//     std::vector<double> prices = {49500, 50500, 51500};
//     MarketDataSimulator sim(engine, "BTCUSD", prices, 2000);
//     sim.start();

//     // 4) Launch the HTTP API in its own thread
//     std::thread apiThread([&](){
//         // Constructor blocks inside listen()
//         ApiServer api(engine, 8081);
//     });

//     std::cout << "Backend running: HTTP API on port 8081, Redis PUB/SUB on 'orders'\n";

//     // 5) Wait here until the API thread ends (e.g. via CTRL+C)
//     apiThread.join();

//     // 6) Clean up simulation
//     sim.stop();
//     return 0;
// }


#include "OrderEngine.hpp"
#include "CommunicationLayer.hpp"
#include "ApiServer.hpp"
#include <hiredis/hiredis.h>
#include "json.hpp"           
#include <thread>
#include <iostream>

void tickSubscriber(OrderEngine& engine) {

    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (!c || c->err) {
        std::cerr << "Redis connect error: "
                  << (c ? c->errstr : "unknown")
                  << std::endl;
        return;
    }

    redisReply* reply = (redisReply*)redisCommand(c, "SUBSCRIBE ticks");
    freeReplyObject(reply);

    while (true) {
        void* raw = nullptr;
        if (redisGetReply(c, &raw) != REDIS_OK || !raw) break;
        reply = static_cast<redisReply*>(raw);


        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
            const char* payload = reply->element[2]->str;
            try {
                auto j = nlohmann::json::parse(payload);
                std::string sym   = j.at("symbol").get<std::string>();
                double      price = j.at("price").get<double>();
                engine.onMarketTick(sym, price);
            } catch (std::exception& e) {
                std::cerr << "Tick parse error: " << e.what() << std::endl;
            }
        }

        freeReplyObject(reply);
    }

    redisFree(c);
}

int main() {

    CommunicationLayer comm;
    OrderEngine       engine(comm);

    Order buy  {"1", "BTCUSDT", Side::BUY,  50000.0, 1.0, Type::LIMIT, Status::OPEN};
    Order sell {"2", "BTCUSDT", Side::SELL, 51000.0, 1.0, Type::LIMIT, Status::OPEN};
    engine.placeOCOOrder("oco-1", {buy, sell});
    std::cout << "Placed demo OCO orders\n";

    std::thread tickThread(tickSubscriber, std::ref(engine));

    std::thread apiThread([&](){
        ApiServer api(engine, 8081);
    });

    std::cout << "Backend running:\n"
              << " • HTTP API on port 8081\n"
              << " • Redis PUB/SUB on 'orders' (for order events)\n"
              << " • Redis SUBSCRIBE 'ticks' (for market ticks)\n";

    apiThread.join();

    tickThread.detach();
    return 0;
}
