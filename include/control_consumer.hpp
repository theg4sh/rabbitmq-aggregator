#ifndef CONTROL_CONSUMER_HPP
#define CONTROL_CONSUMER_HPP

#include <memory>
#include <amqp.h>

#include "connection.hpp"

class ControlConsumer
{
public:
    virtual amqp_channel_t channelId() = 0;
    virtual bool get() = 0;

    virtual bool ack(uint64_t deliveryTag) = 0;
    virtual bool reject(uint64_t deliveryTag) = 0;

    virtual std::string getQueueName() = 0;
    virtual std::shared_ptr<Connection> connection() = 0;

    virtual ~ControlConsumer() = default;
};

#endif
