#ifndef OPERATION_HPP
#define OPERATION_HPP

#include <iostream>
#include <memory>
#include <thread>
#include <queue>
#include <stdlib.h>
#include <string.h>

#include "utils.hpp"
#include "diag.hpp"
#include "publisher.hpp"
#include "control_consumer.hpp"

class Operation
{
public:
    enum Status {
        IDLE,
        GET_QUEUE_A,
        WAIT_QUEUE_A,
        GET_QUEUE_B,
        WAIT_QUEUE_B,
        PUBLISH_QUEUE_C,
    };
private:
    struct QueueData {
        uint64_t deliveryTag;
        std::string body;
    };
    struct OperationData {
        QueueData dataA;
        QueueData dataB;
    };

    std::shared_ptr<ControlConsumer> consumerACtl;
    std::shared_ptr<ControlConsumer> consumerBCtl;
    std::shared_ptr<Publisher> publisher;

    Status _status = IDLE;
    bool _isRunning = false;

    std::queue<QueueData> queueA;
    std::queue<QueueData> queueB;

    Diag diag;
public:
    Operation(bool withDiag=false):
        diag(withDiag)
    {
       diag.registerCounter("qA"); // QueueA received
       diag.registerCounter("qB"); // QueueB received
       diag.registerCounter("Pub"); // Published
       //diag.registerCounter("Flt"); // Failts
    }

    void init(std::shared_ptr<ControlConsumer> consumerACtl,
              std::shared_ptr<ControlConsumer> consumerBCtl,
              std::shared_ptr<Publisher> publisher)
    {
        this->consumerACtl = consumerACtl;
        this->consumerBCtl = consumerBCtl;
        this->publisher = publisher;
    }

    void onMessage(const amqp_channel_t channelId,
                   uint64_t deliveryTag,
                   const amqp_bytes_t& /*exchange*/,
                   const amqp_bytes_t& /*routingKey*/,
                   const amqp_message_t& message)
    {
        QueueData qd;
        qd.deliveryTag = deliveryTag;
        qd.body.resize(message.body.len);
        memcpy((void*)qd.body.data(), message.body.bytes, qd.body.size());

        if (channelId == this->consumerACtl->channelId()) {
            this->queueA.push(qd);
            this->diag.counterInc("qA");
        }
        else if (channelId == this->consumerBCtl->channelId()) {
            this->queueB.push(qd);
            this->diag.counterInc("qB");
        }
        else {
            std::cerr << "Unknown channel id: " << channelId << std::endl;
        }
    }

    bool fillOperationData(OperationData& odata)
    {
        if (this->queueA.size()>0 && this->queueB.size()>0) {
            odata.dataA = this->queueA.front(); this->queueA.pop();
            odata.dataB = this->queueB.front(); this->queueB.pop();
            return true;
        }
        return false;
    }

    virtual bool processData(OperationData& odata)
    {
        std::string pubData = odata.dataA.body + odata.dataB.body;
        return this->publisher->publish(pubData);
    }

    void run() {
        this->_isRunning = true;
        this->_status = GET_QUEUE_A;
        this->diag.reset();

        Status failOnceAt = IDLE;
        for(; this->_isRunning; ) {
            this->consumerACtl->connection()->heartbeat();
            this->consumerBCtl->connection()->heartbeat();
            this->publisher->connection()->heartbeat();

            this->diag.report();
            switch(this->_status) {
            case IDLE:
                std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                continue;
            case GET_QUEUE_A:
                if (this->queueA.empty()) {
                    if (!this->consumerACtl->get()) {
                        if (failOnceAt != this->_status) {
                            std::cerr << "Failed to get message from queue A. Will try repeat continuously" << std::endl;
                            failOnceAt = this->_status;
                        }
                        std::this_thread::sleep_for( std::chrono::milliseconds(100) );
                    }
                    continue;
                }
                this->_status = WAIT_QUEUE_A;
                break;
            case WAIT_QUEUE_A:
                if (this->queueA.empty()) {
                    std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                    continue;
                }
                this->_status = GET_QUEUE_B;
                // FALLTHOUGTH
            case GET_QUEUE_B:
                if (this->queueB.empty()) {
                    if (!this->consumerBCtl->get()) {
                        if (failOnceAt != this->_status) {
                            std::cerr << "Failed to get message from queue B. Will try repeat continuously" << std::endl;
                            failOnceAt = this->_status;
                        }
                        std::this_thread::sleep_for( std::chrono::milliseconds(100) );
                    }
                    continue;
                }
                this->_status = WAIT_QUEUE_B;
                break;
            case WAIT_QUEUE_B:
                if (this->queueB.empty()) {
                    std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                    continue;
                }
                this->_status = PUBLISH_QUEUE_C;
                // FALLTHOUGTH
            case PUBLISH_QUEUE_C:
                OperationData odata;
                if (this->fillOperationData(odata)) {
                    if (this->processData(odata)) {
                        this->diag.counterInc("Pub");
                        //std::cerr << "Acking queued data" << std::endl;
                        this->consumerACtl->ack(odata.dataA.deliveryTag);
                        this->consumerBCtl->ack(odata.dataB.deliveryTag);
                    } else {
                        std::cerr << "Rejecting queued data" << std::endl;
                        this->consumerACtl->reject(odata.dataA.deliveryTag);
                        this->consumerACtl->reject(odata.dataB.deliveryTag);
                    }
                    this->_status = GET_QUEUE_A;
                } else {
                    std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                }
            }
            //
        }
    }
};

#endif
