#include "publisher.hpp"

Publisher::Publisher(std::shared_ptr<Connection> connection,
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

Publisher::~Publisher() {
    amqp_bytes_free(_exchange);
    amqp_bytes_free(_routingKey);
}

std::shared_ptr<Connection> Publisher::connection()
{
    return this->_connection;
}

bool Publisher::publish(std::string body)
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

