#include "channel.hpp"
#include "connection.hpp"

Channel::Status Channel::status() const
{
    return this->_status;
}

std::string Channel::statusStr() const
{
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

bool Channel::open() {
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

amqp_channel_t Channel::get()
{
    return this->channelId;
}

void Channel::close()
{
    if (this->_status == OPEN) {
        amqp_channel_close(this->_connection->get(),
                           this->channelId, AMQP_REPLY_SUCCESS);
    }
}

Channel::~Channel()
{
    this->close();
}
