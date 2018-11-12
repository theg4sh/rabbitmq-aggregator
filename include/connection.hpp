#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include "utils.hpp"
#include "resolver.hpp"
#include "error.hpp"

class Connection
{
public:
    enum Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };

private:
    std::string connectionUri;
    AmqpResolver resolver;

    amqp_socket_t *socket = NULL;
    amqp_connection_state_t conn;

    // TODO async queries
    //std::queue<int> waiters;

    Status _status = DISCONNECTED;

    uint64_t lastHeartbeat = 0;
public:
    Connection(const std::string& connectionUri):
        connectionUri(connectionUri) {}

    bool connect()
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
        if (isAmqpError(reply)) {
            this->close();
            return false;
        }

        this->_status = CONNECTED;
        return true;
    }

    Status status() { return this->_status; }
    std::string statusStr() {
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

    amqp_connection_state_t get() { return this->conn; }

    bool heartbeat()
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

    void close() {
        if (this->_status != DISCONNECTED) {
            this->_status = DISCONNECTING;
            if (this->conn) {
                amqp_connection_close(this->conn, AMQP_REPLY_SUCCESS);
            }
            this->_status = DISCONNECTED;
        }
        this->lastHeartbeat = 0;
    }

    ~Connection() {
        this->close();
        amqp_destroy_connection(this->conn);
    }
};

#endif
