#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <memory>

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include "utils.hpp"
#include "resolver.hpp"
#include "error.hpp"
#include "channel.hpp"

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    enum Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };

private:
    std::vector<std::shared_ptr<Channel>> channels;

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

    bool connect();

    Status status();
    std::string statusStr();

    std::string getSafeConnectionInfo();

    amqp_connection_state_t get();

    std::shared_ptr<Channel> createChannel();

    bool heartbeat();

    void close();

    ~Connection();
};

#endif
