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
             amqp_boolean_t noAck):
        _isRunning(false),
        _connection(connection),
        _channel(channel),
        queueName(queueName),
        noAck(noAck)
    {
        queue = amqp_bytes_malloc(this->queueName.size());
        memcpy(queue.bytes, this->queueName.data(), this->queueName.size());
    }

    virtual ~Consumer() {
        amqp_bytes_free(queue);
    }

    void stop() {
        if (this->_status != IDLE) {
            this->_status = STOPPING;
        }
    }

    virtual void onMessage(uint64_t deliveryTag,
                   const amqp_bytes_t& exchange,
                   const amqp_bytes_t& routingKey,
                   const amqp_message_t& message)
    {
        std::string body;
        body.resize(message.body.len);
        memcpy((void*)body.data(), message.body.bytes, body.size());
        std::cout << "Message received: " << deliveryTag 
            << " Excahnge: " << (const char*)exchange.bytes
            << " RoutingKey: " << (const char*)routingKey.bytes
            << " Message: " << body << std::endl;
        this->ack(deliveryTag);
    }

    virtual amqp_channel_t channelId() override { return this->_channel->get(); }
    virtual std::shared_ptr<Connection> connection() override { return this->_connection; }

    virtual bool ack(uint64_t deliveryTag) override {
        int ret = amqp_basic_ack(this->_connection->get(),
                                 this->_channel->get(),
                                 deliveryTag, false);
        if (ret) {
            std::cerr << "Failed to ack message: " << deliveryTag << std::endl;
            return false;
        }
        return true;
    }


    virtual bool reject(uint64_t deliveryTag) override {
        int ret = amqp_basic_reject(this->_connection->get(),
                                    this->_channel->get(),
                                    deliveryTag, false);
        if (ret) {
            std::cerr << "Failed to ack message: " << deliveryTag << std::endl;
            return false;
        }
        return true;
    }

    virtual bool get() override
    {
        this->_status = START_CONSUMING;
        if (this->_connection->status() != Connection::Status::CONNECTED) {
            std::cerr << "Connection status: " << this->_connection->statusStr() << std::endl;
            return false;
        }
        if (this->_channel->status() != Channel::Status::OPEN) {
            std::cerr << "Channel status: " << this->_channel->statusStr() << std::endl;
            return false;
        }

        amqp_rpc_reply_t ret = amqp_basic_get(this->_connection->get(),
                                              this->_channel->get(),
                                              this->queue, this->noAck);
        if (isAmqpError(ret)) {
            std::cerr << "Basic get - ReplyType: " << ret.reply_type 
                << "ReplyID: " << ret.reply.id << std::endl;
            return false;
        }

        if (ret.reply_type == AMQP_RESPONSE_NORMAL
            && ret.reply.id == AMQP_BASIC_GET_OK_METHOD) {
            auto x = (amqp_basic_get_ok_t*) ret.reply.decoded;
            {
                amqp_message_t message;
                ret = amqp_read_message(this->_connection->get(), this->_channel->get(), &message, 0);
                if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                    std::cerr << "Read message - ReplyType: " << ret.reply_type 
                        << "ReplyID: " << ret.reply.id << std::endl;
                    return false;
                }
                if (isAmqpError(ret)) {
                    std::cerr << "Read message - ReplyType: " << ret.reply_type 
                        << "ReplyID: " << ret.reply.id << std::endl;
                    return false;
                }
                this->onMessage(x->delivery_tag, x->exchange, x->routing_key, message);
                amqp_destroy_message(&message);
            }
            return true;
        }
        isAmqpError(ret);
        return false;
    }
};

#endif
