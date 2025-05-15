#include "CommunicationLayer.hpp"
#include <iostream>

CommunicationLayer::CommunicationLayer(const std::string& host, int port) {
    context_ = redisConnect(host.c_str(), port);
    if (!context_ || context_->err) {
        if (context_) {
            std::cerr << "Redis connection error: " << context_->errstr << "\n";
            redisFree(context_);
        }
        context_ = nullptr;
    }
}

CommunicationLayer::~CommunicationLayer() {
    if (context_) redisFree(context_);
}

void CommunicationLayer::publish(const std::string& channel, const std::string& message) {
    if (!context_) return;
    redisReply* reply = (redisReply*)redisCommand(context_,
                                "PUBLISH %s %s",
                                channel.c_str(), message.c_str());
    if (!reply) {
        std::cerr << "Redis publish failed for: " << message << "\n";
        return;
    }
    freeReplyObject(reply);
}
