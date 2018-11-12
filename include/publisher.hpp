#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP

#include <memory>
#include <string.h>

#include <amqp.h>

#include "connection.hpp"
#include "channel.hpp"

class Publisher
{
private:
    std::shared_ptr<Connection> _connection;
    std::shared_ptr<Channel> _channel;
    std::string exchange;
    std::string routingKey;

    amqp_bytes_t _exchange;
    amqp_bytes_t _routingKey;
    /**
     * @param _mandatory indicate to the broker that the message 
     * MUST be routed to a queue. If the broker cannot do this
     * it should respond with a basic.reject method.
     */
    int _mandatory = 1;
    /**
     * @param _immediate indicate to the broker that the message
     * MUST be delivered to a consumer immediately. If the broker
     * cannot do this it should response with a basic.reject method. 
     */
    int _immediate = 0;
public:
    Publisher(std::shared_ptr<Connection> connection,
              std::shared_ptr<Channel> channel,
              std::string exchange,
              std::string routingKey):
        _connection(connection),
        _channel(channel),
        exchange(exchange),
        routingKey(routingKey)
    {
        _exchange = amqp_bytes_malloc(exchange.size());
        memcpy(_exchange.bytes, (void*)exchange.data(), exchange.size());
        std::cerr << "Publish exchange: " << (const char*)_exchange.bytes << std::endl;

        _routingKey = amqp_bytes_malloc(routingKey.size());
        memcpy(_routingKey.bytes, (void*)routingKey.data(), routingKey.size());
        std::cerr << "Publish routing key: " << (const char*)_routingKey.bytes << std::endl;
    }
    
    ~Publisher() {
        amqp_bytes_free(_exchange);
        amqp_bytes_free(_routingKey);
    }

    std::shared_ptr<Connection> connection() { return this->_connection; }

    virtual bool publish(std::string body)
    {
        amqp_bytes_t _body = amqp_bytes_malloc(body.size());
        memcpy(_body.bytes, (void*)body.data(), body.size());

        int res = amqp_basic_publish(this->_connection->get(),
                                     this->_channel->get(),
                                     this->_exchange, this->_routingKey,
                                     this->_mandatory, this->_immediate, NULL/*properties*/,
                                     _body);
        amqp_bytes_free(_body);
        if (res>0) {
            return false;
        }
        return true;
    }
};

#endif
