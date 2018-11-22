#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>

/**
 * @brief Config uses in project to configure connections and settings for queues,
 * but also it describes default values for some parameters.
 */
struct QueueConfig {
    std::string diagTitle;
    std::string connectionUri;
    std::string name;
    std::string exchange;
    std::string routingKey;
};

struct Config {
    /**
     * @param enableDiag print out every second statistics by processed messages
     */
    bool enableDiag = false;

    std::string connectionUri;

    std::vector<QueueConfig> consumers;

    QueueConfig publisher;
};

extern Config cfg;

#endif

