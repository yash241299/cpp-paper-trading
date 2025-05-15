#ifndef COMMUNICATION_LAYER_HPP
#define COMMUNICATION_LAYER_HPP

#include <string>
#include <hiredis/hiredis.h>

class CommunicationLayer {
public:
    CommunicationLayer(const std::string& host = "127.0.0.1", int port = 6379);
    ~CommunicationLayer();
    virtual void publish(const std::string& channel, const std::string& message);

private:
    redisContext* context_;
};

#endif
