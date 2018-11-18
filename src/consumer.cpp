#include "consumer.hpp"

Consumer::Consumer(std::shared_ptr<Connection> connection,
                   std::shared_ptr<Channel> channel,
                   std::string queueName,
                   amqp_boolean_t noAck):
    _isRunning(false),
    _connection(connection),
    _channel(channel),
    queueName(queueName),
    noAck(noAck)
{
    queue = amqp_bytes_malloc(this->queueName.size());
    memcpy(queue.bytes, this->queueName.data(), this->queueName.size());
}

Consumer::~Consumer() {
    amqp_bytes_free(queue);
}


void Consumer::stop()
{
    if (this->_status != IDLE) {
        this->_status = STOPPING;
    }
}

void Consumer::onMessage(uint64_t deliveryTag,
               const amqp_bytes_t& exchange,
               const amqp_bytes_t& routingKey,
               const amqp_message_t& message)
{
    std::string body;
    body.resize(message.body.len);
    memcpy((void*)body.data(), message.body.bytes, body.size());
    std::cout << "Message received: " << deliveryTag 
        << " Excahnge: " << (const char*)exchange.bytes
        << " RoutingKey: " << (const char*)routingKey.bytes
        << " Message: " << body << std::endl;
    this->ack(deliveryTag);
}

amqp_channel_t Consumer::channelId()
{
    return this->_channel->get();
}

std::string Consumer::getQueueName()
{
    return this->queueName;
}

std::shared_ptr<Connection> Consumer::connection()
{
    return this->_connection;
}

bool Consumer::ack(uint64_t deliveryTag)
{
    int ret = amqp_basic_ack(this->_connection->get(),
                             this->_channel->get(),
                             deliveryTag, false);
    if (ret) {
        std::cerr << "Failed to ack message: " << deliveryTag << std::endl;
        return false;
    }
    return true;
}


bool Consumer::reject(uint64_t deliveryTag)
{
    int ret = amqp_basic_reject(this->_connection->get(),
                                this->_channel->get(),
                                deliveryTag, false);
    if (ret) {
        std::cerr << "Failed to ack message: " << deliveryTag << std::endl;
        return false;
    }
    return true;
}

bool Consumer::get()
{
    this->_status = START_CONSUMING;
    if (this->_connection->status() != Connection::Status::CONNECTED) {
        std::cerr << "Connection status: " << this->_connection->statusStr() << std::endl;
        return false;
    }
    if (this->_channel->status() != Channel::Status::OPEN) {
        std::cerr << "Channel status: " << this->_channel->statusStr() << std::endl;
        return false;
    }

    amqp_rpc_reply_t ret = amqp_basic_get(this->_connection->get(),
                                          this->_channel->get(),
                                          this->queue, this->noAck);
    if (isAmqpError(ret)) {
        std::cerr << "Basic get - ReplyType: " << ret.reply_type 
            << "ReplyID: " << ret.reply.id << std::endl;
        return false;
    }

    if (ret.reply_type == AMQP_RESPONSE_NORMAL
        && ret.reply.id == AMQP_BASIC_GET_OK_METHOD) {
        auto x = (amqp_basic_get_ok_t*) ret.reply.decoded;
        {
            amqp_message_t message;
            ret = amqp_read_message(this->_connection->get(), this->_channel->get(), &message, 0);
            if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                std::cerr << "Read message - ReplyType: " << ret.reply_type 
                    << "ReplyID: " << ret.reply.id << std::endl;
                return false;
            }
            if (isAmqpError(ret)) {
                std::cerr << "Read message - ReplyType: " << ret.reply_type 
                    << "ReplyID: " << ret.reply.id << std::endl;
                return false;
            }
            this->onMessage(x->delivery_tag, x->exchange, x->routing_key, message);
            amqp_destroy_message(&message);
        }
        return true;
    }
    isAmqpError(ret);
    return false;
}
