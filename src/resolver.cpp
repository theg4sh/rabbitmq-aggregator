#include "resolver.hpp"

//#define DEBUG

AmqpResolver::AmqpResolver():
    valid(false) {}

AmqpResolver::AmqpResolver(const std::string& connectionUri):
    valid(false),
    connectionUri(connectionUri),
    _port(5672),
    _vhost("/")
{
    char connUri[connectionUri.size()];
    memcpy(connUri, connectionUri.data(), connectionUri.size());
    int res = amqp_parse_url((char*)connectionUri.c_str(), &this->parsed);
    this->isParsedValid = (res == 0);
}

bool AmqpResolver::verify()
{
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
#   ifdef DEBUG
    std::cout << this->protocol() << " " << this->user() << " : " << this->pass()
        << " @ " << this->host() << " : " << this->port() << " '" << this->vhost() << "'" << std::endl;
#   endif
    return this->isParsedValid;
}

bool AmqpResolver::isValid()
{
    return this->valid;
}

std::string AmqpResolver::protocol() const
{
    return this->_protocol;
}

std::string AmqpResolver::host() const
{
    return this->_host;
}

std::string AmqpResolver::user() const
{
    return this->_user;
}

std::string AmqpResolver::pass() const
{
    return this->_pass;
}

int AmqpResolver::port() const
{
    return this->_port;
}

std::string AmqpResolver::vhost() const
{
    return this->_vhost;
}

