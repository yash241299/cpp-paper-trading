#include "OrderEngine.hpp"
#include "CommunicationLayer.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <string>

// A fake communicator that collects published JSON
struct FakeComm : public CommunicationLayer {
    std::vector<std::string> events;

    // Use the default host/port so call base ctor with no args
    FakeComm() : CommunicationLayer() {}

    // Override publish to capture messages instead of sending to Redis
    void publish(const std::string& /*channel*/, const std::string& msg) override {
        events.push_back(msg);
    }
};

TEST(OrderEngine, LimitOrderFillsOnTick) {
    FakeComm comm;
    OrderEngine engine(comm);

    // Place a limit BUY at price 100
    Order o{"1","SYM", Side::BUY, 100.0, 1.0, Type::LIMIT, Status::OPEN};
    engine.placeOrder(o);

    // Should have published exactly one OPEN event
    ASSERT_EQ(comm.events.size(), 1u);
    EXPECT_NE(comm.events[0].find("\"status\":\"OPEN\""), std::string::npos);

    // Tick at 101 → no fill
    engine.onMarketTick("SYM", 101.0);
    EXPECT_EQ(comm.events.size(), 1u);

    // Tick at  99 → should fill
    engine.onMarketTick("SYM", 99.0);
    ASSERT_EQ(comm.events.size(), 2u);
    EXPECT_NE(comm.events[1].find("\"status\":\"FILLED\""), std::string::npos);
}

TEST(OrderEngine, MarketOrderFillsImmediately) {
    FakeComm comm;
    OrderEngine engine(comm);

    // Simulate a prior tick so lastPrice_["BTC"] is set
    engine.onMarketTick("BTC", 500.0);

    // Place a market SELL (price field ignored)
    Order m{"2","BTC", Side::SELL, 0.0, 1.0, Type::MARKET, Status::OPEN};
    engine.placeOrder(m);

    // Should see OPEN + FILLED
    ASSERT_EQ(comm.events.size(), 2u);
    EXPECT_NE(comm.events[0].find("\"status\":\"OPEN\""),   std::string::npos);
    EXPECT_NE(comm.events[1].find("\"status\":\"FILLED\""), std::string::npos);
}
