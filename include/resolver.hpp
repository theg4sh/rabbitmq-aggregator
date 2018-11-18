#ifndef RESOLVER_HPP
#define RESOLVER_HPP

#include <iostream>
#include <string>

//#define DEBUG

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
    AmqpResolver():
        valid(false) {}

    AmqpResolver(const std::string& connectionUri):
        valid(false),
        connectionUri(connectionUri),
        _port(5672),
        _vhost("/")
    {
        char connUri[connectionUri.size()];
        memcpy(connUri, connectionUri.data(), connectionUri.size());
        int res = amqp_parse_url((char*)connectionUri.c_str(), &this->parsed);
        std::cerr << connectionUri << std::endl;
        this->isParsedValid = (res == 0);
    }

    bool verify() {
        if (this->isParsedValid) {
            this->_protocol = (this->parsed.ssl ? "amqps" : "amqp");
            this->_host = this->parsed.host;
            this->_user = this->parsed.user;
            this->_pass = this->parsed.password;
            this->_port = this->parsed.port;
            if (strlen(this->parsed.vhost) > 0) {
                this->_vhost = this->parsed.vhost;
            }
        }
#       ifdef DEBUG
        std::cout << this->protocol() << " " << this->user() << " : " << this->pass()
            << " @ " << this->host() << " : " << this->port() << " '" << this->vhost() << "'" << std::endl;
#       endif
        return this->isParsedValid;
    }

    bool isValid() { return this->valid; }

    std::string protocol() const { return this->_protocol; }

    std::string host() const { return this->_host; }
    std::string user() const { return this->_user; }
    std::string pass() const { return this->_pass; }
    int         port() const { return this->_port; }

    std::string vhost() const { return this->_vhost; }

};

#endif
