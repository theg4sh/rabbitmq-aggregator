#include <memory>
#include "config.hpp"
#include "connection.hpp"
#include "channel.hpp"
#include "consumer.hpp"
#include "operation.hpp"

class QueueConsumer : public Consumer
{
private:
    Operation& operation;
public:
    QueueConsumer(Operation& operation,
                  std::shared_ptr<Connection> connection,
                  std::shared_ptr<Channel> channel,
                  std::string queueName,
                  amqp_boolean_t noAck):
        Consumer(connection, channel, queueName, noAck),
        operation(operation) {}

    virtual void onMessage(uint64_t deliveryTag, const amqp_bytes_t& exchange,
                   const amqp_bytes_t& routingKey, const amqp_message_t& message) override
    {
        this->operation.onMessage(this->Consumer::channelId(), deliveryTag, exchange,
                                   routingKey, message);
    }
};

int main(int, char**)
{
    Config cfg;

    auto connectionA = std::make_shared<Connection>(cfg.mqConsumerConnectionUri);
    if (!connectionA->connect()) {
        std::cerr << "Connection A failed" << std::endl;
        return 1;
    }
    auto channelA = connectionA->createChannel();
    if (!channelA->open()) {
        std::cerr << "Failed to open channel" << std::endl;
        return 2;
    }
    std::cout << "Connected A." << std::endl;

    auto connectionB = std::make_shared<Connection>(cfg.mqConsumerConnectionUri);
    if (!connectionB->connect()) {
        std::cerr << "Connection B failed" << std::endl;
        return 1;
    }
    auto channelB = connectionB->createChannel();
    if (!channelB->open()) {
        std::cerr << "Failed to open channel" << std::endl;
        return 2;
    }
    std::cout << "Connected B." << std::endl;

    auto connectionC = std::make_shared<Connection>(cfg.mqPublisherConnectionUri);
    if (!connectionC->connect()) {
        std::cerr << "Connection C failed" << std::endl;
        return 1;
    }
    auto channelC = connectionC->createChannel();
    if (!channelC->open()) {
        std::cerr << "Failed to open channel" << std::endl;
        return 2;
    }
    std::cout << "Connected C." << std::endl;

    std::cout << "Channel-A status: " << channelA->statusStr() << std::endl;
    std::cout << "Channel-B status: " << channelB->statusStr() << std::endl;
    std::cout << "Channel-C status: " << channelC->statusStr() << std::endl;

    Operation operation(cfg.enableDiag);

    std::vector<std::shared_ptr<ControlConsumer>> consumers;
    consumers.push_back(
            std::make_shared<QueueConsumer>(operation, connectionA, channelA, cfg.qA.qName, false));
    consumers.push_back(
            std::make_shared<QueueConsumer>(operation, connectionB, channelB, cfg.qB.qName, false));

    auto publisher = std::make_shared<Publisher>
            (connectionC, channelC, cfg.qPublisher.qExchange, cfg.qPublisher.qRoutingKey);

    operation.init(consumers, publisher);

    operation.run();
    return 0;
}
