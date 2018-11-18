#include "operation.hpp"

Operation::Operation(bool withDiag):
        diag(withDiag) {}

void Operation::init(std::vector<std::shared_ptr<ControlConsumer>> consumers,
          std::shared_ptr<Publisher> publisher)
{
    this->consumers = consumers;
    this->queues.resize(consumers.size());
    this->publisher = publisher;

    for (auto& consumer: this->consumers) {
        diag.registerCounter(consumer->getQueueName().c_str());
    }
    diag.registerCounter("Pub"); // Published
}

void Operation::onMessage(const amqp_channel_t channelId,
               uint64_t deliveryTag,
               const amqp_bytes_t& /*exchange*/,
               const amqp_bytes_t& /*routingKey*/,
               const amqp_message_t& message)
{
    QueueData qd;
    qd.deliveryTag = deliveryTag;
    qd.body.resize(message.body.len);
    memcpy((void*)qd.body.data(), message.body.bytes, qd.body.size());

    if (channelId == this->consumers[this->currentConsumer]->channelId()) {
        this->queues[this->currentConsumer].push(qd);
        this->diag.counterInc(this->consumers[this->currentConsumer]->getQueueName().c_str());
    }
    else {
        std::cerr << "Unknown channel id: " << channelId << std::endl;
    }
}

bool Operation::fillOperationData(OperationData& odata)
{
    for (auto& q : this->queues) {
        if (q.size() == 0) {
            return false;
        }
    }
    for (auto& q : this->queues) {
        odata.data.push_back(q.front()); q.pop();
    }
    return true;
}

bool Operation::processData(OperationData& odata)
{
    std::string pubData;
    for (auto& qd : odata.data) {
        pubData += qd.body;
    }
    return this->publisher->publish(pubData);
}

void Operation::run()
{
    this->_isRunning = true;
    this->_status = GET_QUEUE;
    this->currentConsumer = 0;
    this->diag.reset();

    Status failOnceAt = IDLE;
    for(; this->_isRunning; ) {
        for (auto& consumer : this->consumers) {
            consumer->connection()->heartbeat();
        }
        this->publisher->connection()->heartbeat();

        this->diag.report();
        switch(this->_status) {
        case IDLE:
            std::this_thread::sleep_for( std::chrono::milliseconds(10) );
            continue;
        case GET_QUEUE:
            if (this->queues[this->currentConsumer].empty()) {
                if (!this->consumers[this->currentConsumer]->get()) {
                    if (failOnceAt != this->_status) {
                        std::cerr << "Failed to get message from queue "
                            << this->consumers[this->currentConsumer]->getQueueName()
                            << ". Will try repeat continuously" << std::endl;
                        failOnceAt = this->_status;
                    }
                    std::this_thread::sleep_for( std::chrono::milliseconds(100) );
                }
                continue;
            }
            this->_status = WAIT_QUEUE;
            break;
        case WAIT_QUEUE:
            if (this->queues[this->currentConsumer].empty()) {
                std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                continue;
            }

            this->currentConsumer++;
            if (this->currentConsumer < this->consumers.size()) {
                this->_status = GET_QUEUE;
                break;
            }

            this->_status = PUBLISH_QUEUE;
            // FALLTHOUGTH
        case PUBLISH_QUEUE:
            OperationData odata;
            if (this->fillOperationData(odata)) {
                if (this->processData(odata)) {
                    this->diag.counterInc("Pub");
                    //std::cerr << "Acking queued data" << std::endl;
                    for (std::size_t it=0; it < this->consumers.size(); it++) {
                        auto deliveryTag = odata.data[it].deliveryTag;
                        this->consumers[it]->ack(deliveryTag);
                    }
                } else {
                    std::cerr << "Rejecting queued data" << std::endl;
                    for (std::size_t it=0; it < this->consumers.size(); it++) {
                        auto deliveryTag = odata.data[it].deliveryTag;
                        this->consumers[it]->reject(deliveryTag);
                    }
                }
                this->currentConsumer = 0;
                this->_status = GET_QUEUE;
            } else {
                std::this_thread::sleep_for( std::chrono::milliseconds(10) );
            }
        }
        //
    }
}

