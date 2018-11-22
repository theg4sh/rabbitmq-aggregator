#ifndef ERROR_HPP
#define ERROR_HPP

#include <amqp.h>

bool isAmqpErrorWithMsg(const amqp_rpc_reply_t& x);

bool isAmqpConnectionCloseError(const amqp_rpc_reply_t& x);

bool isAmqpChannelCloseError(const amqp_rpc_reply_t& x);

#endif
