#include "OrderEngine.hpp"
#include <sstream>
#include <algorithm>

OrderEngine::OrderEngine(CommunicationLayer& comm)
    : comm_(comm)
{}

OrderEngine::~OrderEngine() = default;

void OrderEngine::placeOrder(const Order& order) {
    // 1. Store
    orders_[order.id] = order;

    // 2. Publish OPEN + type
    {
        std::ostringstream os;
        os << "{"
           << "\"id\":\""     << order.id   << "\","
           << "\"status\":\"" << "OPEN"     << "\","
           << "\"type\":\""
           << (order.type == Type::MARKET ? "MARKET" : "LIMIT")
           << "\""
           << "}";
        publishEvent(os.str());
    }

    // 3. If MARKET, -> immediate fill
    if (order.type == Type::MARKET) {
        auto it = lastPrice_.find(order.symbol);
        if (it != lastPrice_.end()) {
            matchOrder(orders_[order.id], it->second);
        }
    }
}

void OrderEngine::placeOCOOrder(const std::string& ocoId,
                                const std::vector<Order>& orders) {
    std::vector<std::string> ids;
    for (const auto& o : orders) {
        // store
        orders_[o.id] = o;
        ids.push_back(o.id);

        {
            std::ostringstream os;
            os << "{"
               << "\"id\":\""     << o.id   << "\","
               << "\"status\":\"" << "OPEN"  << "\","
               << "\"type\":\""
               << (o.type == Type::MARKET ? "MARKET" : "LIMIT")
               << "\""
               << "}";
            publishEvent(os.str());
        }

        if (o.type == Type::MARKET) {
            auto it = lastPrice_.find(o.symbol);
            if (it != lastPrice_.end()) {
                matchOrder(orders_[o.id], it->second);
            }
        }
    }
    ocoGroups_[ocoId] = std::move(ids);
}

void OrderEngine::onMarketTick(const std::string& symbol, double price) {

    {
        std::ostringstream tickJson;
            tickJson << "{"
               << "\"event\":\"TICK\","
                << "\"symbol\":\"" << symbol << "\","
                << "\"price\":"    << price << ","
                << "\"time\":"     << std::chrono::duration_cast<std::chrono::milliseconds>(
                                             std::chrono::system_clock::now().time_since_epoch()
                                           ).count()
                << "}";
            publishEvent(tickJson.str());
    }

    lastPrice_[symbol] = price;

    for (auto& [_, order] : orders_) {
        if (order.symbol == symbol &&
            order.status == Status::OPEN &&
            order.type   == Type::LIMIT) {
            matchOrder(order, price);
        }
    }
}

void OrderEngine::matchOrder(Order& order, double marketPrice) {
    bool shouldFill = (order.side == Side::BUY  && marketPrice <= order.price)
                   || (order.side == Side::SELL && marketPrice >= order.price);

    if (!shouldFill) return;

    order.status = Status::FILLED;

    for (auto& [_, ids] : ocoGroups_) {
        if (std::find(ids.begin(), ids.end(), order.id) != ids.end()) {
            for (const auto& otherId : ids) {
                if (otherId != order.id) cancelOrder(otherId);
            }
            break;
        }
    }

    {
        std::ostringstream os;
        os << "{"
           << "\"id\":\""     << order.id   << "\","
           << "\"status\":\"" << "FILLED"   << "\","
           << "\"type\":\""
           << (order.type == Type::MARKET ? "MARKET" : "LIMIT")
           << "\""
           << "}";
        publishEvent(os.str());
    }
}

void OrderEngine::cancelOrder(const std::string& orderId) {
    auto it = orders_.find(orderId);
    if (it != orders_.end() && it->second.status == Status::OPEN) {
        it->second.status = Status::CANCELED;
        {
            const auto& o = it->second;
            std::ostringstream os;
            os << "{"
               << "\"id\":\""     << orderId    << "\","
               << "\"status\":\"" << "CANCELED" << "\","
               << "\"type\":\""
               << (o.type == Type::MARKET ? "MARKET" : "LIMIT")
               << "\""
               << "}";
            publishEvent(os.str());
        }
    }
}
