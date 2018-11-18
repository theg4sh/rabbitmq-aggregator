#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <memory>
#include <vector>
#include "error.hpp"

class Connection;

class Channel
{
public:
    enum Status {
        CLOSED,
        OPENING,
        OPEN,
        CLOSING,
    };
private:
    std::shared_ptr<Connection> _connection;
    amqp_channel_t channelId;

    Status _status = CLOSED;

public:
    Channel() = delete;

    Channel(std::shared_ptr<Connection> connection, amqp_channel_t& channelId):
        _connection(connection), channelId(channelId) {}

    Status status() const;
    std::string statusStr() const;

    bool open();

    amqp_channel_t get();

    void close();

    ~Channel();
};

#endif
