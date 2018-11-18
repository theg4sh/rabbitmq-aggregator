#ifndef ERROR_HPP
#define ERROR_HPP

#include <amqp.h>

bool isAmqpError(amqp_rpc_reply_t x);

#endif
