#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include "connection.hpp"
#include "error.hpp"

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
    static std::vector<amqp_channel_t> channels;
    std::shared_ptr<Connection> _connection;
    amqp_channel_t channelId;

    Status _status = CLOSED;

public:
    Channel() = delete;

    Channel(std::shared_ptr<Connection>& connection, amqp_channel_t& channelId):
        _connection(connection), channelId(channelId) {}

    static std::shared_ptr<Channel> create(std::shared_ptr<Connection> connection) {
        amqp_channel_t channelId = 1;
        for (const auto chid : Channel::channels) {
            if (channelId <= chid) {
                channelId = chid+1;
            }
        }
        Channel::channels.push_back(channelId);
        return std::make_shared<Channel>(connection, channelId);
    }

    Status status() const { return this->_status; }
    std::string statusStr() const {
        switch (this->_status) {
        case CLOSED:
            return "CLOSED";
        case OPENING:
            return "OPENING";
        case OPEN:
            return "OPEN";
        case CLOSING:
            return "CLOSING";
        }
        return "<UNKNOWN>";
    }

    bool open() {
        if (this->_connection->status() != Connection::Status::CONNECTED) {
            return false;
        }
        this->_status = OPENING;

        amqp_channel_open(this->_connection->get(), this->channelId);
        amqp_rpc_reply_t ret = amqp_get_rpc_reply(this->_connection->get());
        if (isAmqpError(ret)) {
            this->_status = CLOSED;
            return false;
        }
        this->_status = OPEN;
        return true;
    }

    amqp_channel_t get() { return this->channelId; }

    void close() {
        if (this->_status == OPEN) {
            amqp_channel_close(this->_connection->get(),
                               this->channelId, AMQP_REPLY_SUCCESS);
        }
    }

    ~Channel() {
        this->close();
    }
};

#endif
