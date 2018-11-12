#ifndef CONFIG_HPP
#define CONFIG_HPP

struct Config {
    /**
     * @param enableDiag print out every second statistics by processed messages
     */
    bool enableDiag = true;

    const char* mqConsumerConnectionUri = "amqp://guest:guest@localhost:5672/";

    const char* mqPublisherConnectionUri = "amqp://guest:guest@localhost:5672/";

    struct Queue {
        /**
         *
         */
        const char* qName;
        /**
         *
         */
        const char* qExchange;
        /**
         *
         */
        const char* qRoutingKey;
    };

    // Incoming
    Queue qA{"queue-A", "exchange-in", "routing-key.in-A"};
    Queue qB{"queue-B", "exchange-in", "routing-key.in-B"};

    // Outgoing
    Queue qPublisher{"queue-C", "exchange-out", "routing-key.out"};
};

extern Config cfg;

#endif

