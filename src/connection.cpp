#include <sstream>
#include "connection.hpp"

Connection::~Connection()
{
    this->close();
    amqp_destroy_connection(this->conn);
}

Connection::Status Connection::status()
{
    return this->_status;
}

std::string Connection::statusStr()
{
    switch(this->_status) {
    case DISCONNECTED:
        return "DISCONNECTED";
    case CONNECTING:
        return "CONNECTING";
    case CONNECTED:
        return "CONNECTED";
    case DISCONNECTING:
        return "DISCONNECTING";
    }
    return "<UNKNOWN>";
}

std::string Connection::getSafeConnectionInfo()
{
    std::ostringstream oss;
    oss << this->resolver.protocol() << "://" << this->resolver.host() << ":"
        << this->resolver.port() << this->resolver.vhost();
    return oss.str();
}


amqp_connection_state_t Connection::get()
{
    return this->conn;
}

std::shared_ptr<Channel> Connection::createChannel()
{
    amqp_channel_t channelId = 1;
    for (const auto ch : this->channels) {
        if (channelId <= ch->get()) {
            channelId = ch->get()+1;
        }
    }
    auto channel = std::make_shared<Channel>(shared_from_this(), channelId);
    this->channels.push_back(channel);
    return channel;
}

bool Connection::connect()
{
    if (this->_status != DISCONNECTED) {
        return false;
    }
    this->_status = Status::CONNECTING;

    this->resolver = AmqpResolver(this->connectionUri);
    if (!this->resolver.verify()) {
        this->_status = Status::DISCONNECTED;
        return false;
    }
    this->conn = amqp_new_connection();

    this->socket = amqp_tcp_socket_new(this->conn);
    if (!this->socket) {
        std::cerr << "Failed to create new socket" << std::endl;
        this->close();
        return false;
    }

    int status = amqp_socket_open(this->socket, this->resolver.host().c_str(),
                                  this->resolver.port());
    if (status) {
        std::cerr << "Failed to open socket" << std::endl;
        this->close();
        return false;
    }

    amqp_rpc_reply_t reply = amqp_login(this->conn, this->resolver.vhost().c_str(),
                                        AMQP_DEFAULT_MAX_CHANNELS,
                                        AMQP_DEFAULT_FRAME_SIZE,
                                        1/* set 1 per sec, but AMQP_DEFAULT_HEARTBEAT=disabled*/,
                                        AMQP_SASL_METHOD_PLAIN,
                                        this->resolver.user().c_str(),
                                        this->resolver.pass().c_str());
    if (isAmqpErrorWithMsg(reply)) {
        this->close();
        return false;
    }

    this->_status = CONNECTED;
    return true;
}

bool Connection::heartbeat()
{
    if (this->_status != CONNECTED) {
        return false;
    }
    uint64_t now = now_microseconds();
    if (now-lastHeartbeat >= 1000000) {
        lastHeartbeat = now;
        amqp_frame_t hb;
        hb.channel = 0;
        hb.frame_type = AMQP_FRAME_HEARTBEAT;

        int err;
        if ((err = amqp_send_frame(conn, &hb)))
        {
            const char* errstr = amqp_error_string2(err);
            printf("Error is: %s\n", errstr);
            return false;
        }
    }
    return true;
}

void Connection::close()
{
    if (this->_status != DISCONNECTED) {
        this->_status = DISCONNECTING;
        for (auto& ch: this->channels) {
            ch->close();
        }
        this->channels.clear();
        if (this->conn) {
            amqp_connection_close(this->conn, AMQP_REPLY_SUCCESS);
        }
        this->_status = DISCONNECTED;
    }
    this->lastHeartbeat = 0;
}
