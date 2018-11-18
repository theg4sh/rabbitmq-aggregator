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
              std::string routingKey);
    
    ~Publisher();

    std::shared_ptr<Connection> connection();

    virtual bool publish(std::string body);
};

#endif
