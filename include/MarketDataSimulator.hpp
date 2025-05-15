#ifndef MARKET_DATA_SIMULATOR_HPP
#define MARKET_DATA_SIMULATOR_HPP

#include <string>
#include <vector>
#include <thread>
#include "OrderEngine.hpp"

class MarketDataSimulator {
public:
    // engine: your OrderEngine
    // symbol: e.g. "BTCUSD"
    // priceSequence: list of prices to simulate
    // intervalMs: delay between ticks in milliseconds
    MarketDataSimulator(OrderEngine& engine,
                        const std::string& symbol,
                        std::vector<double> priceSequence,
                        int intervalMs);

    ~MarketDataSimulator();
    void start();
    void stop();

private:
    void run();

    OrderEngine&       engine_;
    std::string        symbol_;
    std::vector<double> prices_;
    int                intervalMs_;
    bool               running_;
    std::thread        thread_;
};

#endif
