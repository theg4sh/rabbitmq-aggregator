#ifndef RESOLVER_HPP
#define RESOLVER_HPP

#include <iostream>
#include <string>
#include <regex>

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

public:
    AmqpResolver():
        valid(false) {}

    AmqpResolver(const std::string& connectionUri):
        valid(false),
        connectionUri(connectionUri),
        _port(5672),
        _vhost("/")
    {
        std::regex uriRegex(
            R"(^(([^:\/?#]+):)?(//((([^@\/?#:]+)(:([^@\/?#]*))?@)?([^@\/?#:]*)(:([^\/?#]*))?))?([^?#]*)(\?([^#]*))?(#(.*))?)",
            std::regex::extended);
        std::smatch uriMatchResult;
        if (std::regex_match(connectionUri, uriMatchResult, uriRegex)) {
            this->_protocol = uriMatchResult[2]; // protocol
            this->_user = uriMatchResult[6]; // username
            this->_pass = uriMatchResult[8]; // password
            this->_host = uriMatchResult[9]; // hostname
            std::string port = uriMatchResult[11]; // port
            std::string path = uriMatchResult[12]; // uri

            if (!port.empty()) {
                this->_port = std::stoi(port);
            }
            if (!path.empty()) {
                this->_vhost = std::move(path);
            }

#           ifdef DEBUG
            int c = 0;
            for (const std::string& res : uriMatchResult) {
                std::cout << c++ << ": " << res << std::endl;
            }
#           endif
        }
    }

    bool verify() {
        this->valid = true;
        if (this->_protocol != "amqp") {
            this->valid = false;
        }
        if (this->_port <=0 ) {
            this->valid = false;
            std::cerr << "Invalid port: " << this->_port << std::endl;
        }
        if (this->_host.empty()) {
            this->valid = false;
            std::cerr << "Invalid hostname: '" << this->_host << "'" << std::endl;
        }
        if (this->_vhost.empty()) {
            this->valid = false;
            std::cerr << "Invalid uri: empty vhost" << std::endl;
        }

#       ifdef DEBUG
        std::cout << this->_protocol << " " << this->_user << " : " << this->_pass
            << " @ " << this->_host << " : " << this->_port << " " << this->_vhost << std::endl;
#       endif
        return this->valid;
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
