#ifndef RESOLVER_HPP
#define RESOLVER_HPP

#include <iostream>
#include <string>

#include <string.h>
#include <amqp.h>

class AmqpResolver
{
private:
    bool valid;

    std::string connectionUri;

    std::string _protocol;
    std::string _host;
    std::string _user;
    std::string _pass;
    int _port = 5672;

    std::string _vhost;

    struct amqp_connection_info parsed;
    bool isParsedValid = false;

public:
    AmqpResolver();

    AmqpResolver(const std::string& connectionUri);

    bool verify();

    bool isValid();

    std::string protocol() const;

    std::string host() const;
    std::string user() const;
    std::string pass() const;
    int         port() const;

    std::string vhost() const;

};

#endif
