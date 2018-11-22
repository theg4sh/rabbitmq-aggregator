#include <memory>
#include "config.hpp"
#include "connection.hpp"
#include "channel.hpp"
#include "consumer.hpp"
#include "operation.hpp"
#include "config_loader.hpp"

class ConsumerForOperation : public Consumer
{
private:
    Operation& operation;
public:
    ConsumerForOperation(Operation& operation,
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

typedef std::pair<std::shared_ptr<Connection>, std::shared_ptr<Channel>> ConnectionAndChannel;

ConnectionAndChannel initializeConnection(const std::string& connectionUri, const std::string& name)
{
    auto connection = std::make_shared<Connection>(connectionUri);
    if (!connection->connect()) {
        std::cerr << "Connection '" << name << "' to '"
            << connection->getSafeConnectionInfo() << "' failed" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }
    auto channel = connection->createChannel();
    if (!channel->open()) {
        std::cerr << "Failed to open channel " << name
            << " on connection '" << connection->getSafeConnectionInfo() << "'" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }
    std::cout << "Connected '" << name << "' to '"
        << connection->getSafeConnectionInfo() << "'." << std::endl;

    return std::make_pair(connection, channel);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <config.cfg>" << std::endl;
        return 1;
    }
    Config cfg;
    ConfigLoader cfgload(argv[1]);
    if (!cfgload.parse(cfg)) {
        std::cerr << cfgload.getError() << std::endl;
        return 2;
    }
    std::cerr << "Config has been loaded" << std::endl;

    ConnectionAndChannel cacC = initializeConnection(cfg.publisher.connectionUri,
                                                     cfg.publisher.diagTitle);

    bool isValid = cacC.first != nullptr;
    std::vector<ConnectionAndChannel> consumersConn;
    for (auto& qcfg : cfg.consumers) {
        auto conn = initializeConnection(qcfg.connectionUri, qcfg.diagTitle);
        isValid = isValid && conn.first != nullptr;
        consumersConn.push_back(conn);
    }
    if (!isValid) {
        std::cerr << "An error occurred during attempt to connect consumers or publisher. Exit." << std::endl;
        return 3;
    }

    Operation operation(cfg.enableDiag);

    std::vector<std::shared_ptr<ControlConsumer>> consumers(cfg.consumers.size());
    for (std::size_t i=0; i<cfg.consumers.size(); i++) {
        auto& cac = consumersConn[i];
        auto& qcfg = cfg.consumers[i];
        consumers[i] = std::make_shared<ConsumerForOperation>(operation, cac.first, cac.second, qcfg.name, false);
    }

    auto publisher = std::make_shared<Publisher>
            (cacC.first, cacC.second, cfg.publisher.exchange, cfg.publisher.routingKey);

    operation.init(consumers, publisher);

    operation.run();
    return 0;
}
