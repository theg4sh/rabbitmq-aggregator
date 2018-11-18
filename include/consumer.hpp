#ifndef CONSUMER_HPP
#define CONSUMER_HPP

#include <atomic>
#include <memory>
#include <queue>
#include <string.h>
#include <stdlib.h>

#include <amqp.h>

#include "utils.hpp"
#include "connection.hpp"
#include "channel.hpp"
#include "control_consumer.hpp"

class Consumer : public ControlConsumer
{
public:
    enum Status {
        IDLE,
        START_CONSUMING,
        CONSUME,
        STOPPING,
    };
private:
    std::atomic_bool _isRunning;

    std::shared_ptr<Connection> _connection;
    std::shared_ptr<Channel> _channel;
    std::string queueName;
    amqp_bytes_t queue;
    amqp_boolean_t noAck;


    Status _status = IDLE;

public:
    Consumer(std::shared_ptr<Connection> connection,
             std::shared_ptr<Channel> channel,
             std::string queueName,
             amqp_boolean_t noAck);

    virtual ~Consumer();

    void stop();

    virtual void onMessage(uint64_t deliveryTag,
                   const amqp_bytes_t& exchange,
                   const amqp_bytes_t& routingKey,
                   const amqp_message_t& message);

    virtual amqp_channel_t channelId() override;
    virtual std::string getQueueName() override;

    virtual std::shared_ptr<Connection> connection() override;

    virtual bool ack(uint64_t deliveryTag) override;


    virtual bool reject(uint64_t deliveryTag) override;

    virtual bool get() override;
};

#endif
