#include <iostream>

#include "error.hpp"

bool isAmqpErrorWithMsg(const amqp_rpc_reply_t& x)
{
    switch(x.reply_type) {
    case AMQP_RESPONSE_NORMAL:
        return false;
    case AMQP_RESPONSE_NONE:
        std::cerr << "Response error: missing RPC reply, type!" << std::endl;
        return true;
    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
        std::cerr << "Library exception: " << amqp_error_string2(x.library_error) << std::endl;
        return true;
    case AMQP_RESPONSE_SERVER_EXCEPTION:
        switch (x.reply.id) {
        case AMQP_CONNECTION_CLOSE_METHOD: {
            amqp_connection_close_t* m =
                (amqp_connection_close_t*) x.reply.decoded;
            std::cerr << "Server exception: server connection error "
                << m->reply_code << "h, message: "
                << (char*)m->reply_text.bytes << std::endl;
            break;
        }
        case AMQP_CHANNEL_CLOSE_METHOD: {
            amqp_channel_close_t* m =
                (amqp_channel_close_t*) x.reply.decoded;
            std::cerr << "Server exception: server channel error "
                << m->reply_code << "h, message: "
                << (char*)m->reply_text.bytes << std::endl;
            break;
        }
        default:
            std::cerr << "unknown server error, method id " << x.reply.id << std::endl;
        }
        return true;
    }
    return false;
}

bool isAmqpConnectionCloseError(const amqp_rpc_reply_t& x)
{
    return x.reply_type == AMQP_RESPONSE_SERVER_EXCEPTION && x.reply.id == AMQP_CONNECTION_CLOSE_METHOD;
}

bool isAmqpChannelCloseError(const amqp_rpc_reply_t& x)
{
    return x.reply_type == AMQP_RESPONSE_SERVER_EXCEPTION && x.reply.id == AMQP_CHANNEL_CLOSE_METHOD;
}
