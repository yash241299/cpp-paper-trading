#ifndef ORDER_ENGINE_HPP
#define ORDER_ENGINE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "CommunicationLayer.hpp"

enum class Side   { BUY, SELL };
enum class Type   { MARKET, LIMIT };
enum class Status { OPEN, FILLED, CANCELED };

struct Order {
    std::string id;
    std::string symbol;
    Side        side;
    double      price;
    double      quantity;
    Type        type;
    Status      status;
};

class OrderEngine {
public:
    explicit OrderEngine(CommunicationLayer& comm);
    ~OrderEngine();

    void placeOrder(const Order& order);
    void placeOCOOrder(const std::string& ocoId,
                       const std::vector<Order>& orders);

    void onMarketTick(const std::string& symbol, double price);

private:
    std::unordered_map<std::string, Order>                 orders_;
    std::unordered_map<std::string, std::vector<std::string>> ocoGroups_;
    std::unordered_map<std::string, double>                lastPrice_;
    CommunicationLayer&                                    comm_;

    void matchOrder(Order& order, double marketPrice);
    void cancelOrder(const std::string& orderId);
    void publishEvent(const std::string& eventJson) {
        comm_.publish("orders", eventJson);
    }
};

#endif
