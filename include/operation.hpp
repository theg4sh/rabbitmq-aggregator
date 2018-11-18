#ifndef OPERATION_HPP
#define OPERATION_HPP

#include <iostream>
#include <memory>
#include <thread>
#include <queue>
#include <vector>
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
        GET_QUEUE,
        WAIT_QUEUE,
        PUBLISH_QUEUE,
    };
private:
    struct QueueData {
        uint64_t deliveryTag;
        std::string body;
    };
    struct OperationData {
        std::vector<QueueData> data;
    };

    std::vector<std::shared_ptr<ControlConsumer>> consumers;
    std::size_t currentConsumer = 0;
    std::shared_ptr<Publisher> publisher;

    Status _status = IDLE;
    bool _isRunning = false;

    std::vector<std::queue<QueueData>> queues;

    Diag diag;
public:
    Operation(bool withDiag=false);

    void init(std::vector<std::shared_ptr<ControlConsumer>> consumers,
              std::shared_ptr<Publisher> publisher);

    void onMessage(const amqp_channel_t channelId,
                   uint64_t deliveryTag,
                   const amqp_bytes_t& exchange,
                   const amqp_bytes_t& routingKey,
                   const amqp_message_t& message);

    bool fillOperationData(OperationData& odata);

    virtual bool processData(OperationData& odata);

    void run();
};

#endif
