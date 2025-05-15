#include "MarketDataSimulator.hpp"
#include <chrono>
#include <iostream>

MarketDataSimulator::MarketDataSimulator(OrderEngine& engine,
                                         const std::string& symbol,
                                         std::vector<double> priceSequence,
                                         int intervalMs)
  : engine_(engine),
    symbol_(symbol),
    prices_(std::move(priceSequence)),
    intervalMs_(intervalMs),
    running_(false)
{}

MarketDataSimulator::~MarketDataSimulator() {
    stop();
}

void MarketDataSimulator::start() {
    if (running_) return;
    running_ = true;
    thread_ = std::thread(&MarketDataSimulator::run, this);
}

void MarketDataSimulator::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void MarketDataSimulator::run() {
    for (double price : prices_) {
        if (!running_) break;
        std::cout << "[Simulator] Tick " << symbol_ << " @ " << price << std::endl;
        engine_.onMarketTick(symbol_, price);
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs_));
    }
}
