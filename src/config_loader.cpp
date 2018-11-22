#include <sstream>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>
#include <libconfig.h>
#include "config_loader.hpp"

bool ConfigLoader::parseQueue(config_setting_t* cfgQueue, QueueConfig& queueConfig)
{
    const char* qDiagTitle;
    const char* qConnectionUri;
    const char* qName;
    const char* qExchange;
    const char* qRoutingKey;

    if (!config_setting_lookup_string(cfgQueue, "name", &qName)) {
        this->ossErr << "Group does not contains required attribute `name`";
        return false;
    }
    if (!config_setting_lookup_string(cfgQueue, "exchange", &qExchange)) {
        this->ossErr << "Group does not contains required attribute `exchange`";
        return false;
    }
    if (!config_setting_lookup_string(cfgQueue, "routingKey", &qRoutingKey)) {
        this->ossErr << "Group does not contains required attribute `routingKey`";
        return false;
    }

    if (!config_setting_lookup_string(cfgQueue, "diagTitle", &qDiagTitle)) {
        qDiagTitle = qName;
    }
    if (!config_setting_lookup_string(cfgQueue, "connectionUri", &qConnectionUri)) {
        qConnectionUri = this->defaultConnectionUri;
    }

    queueConfig.diagTitle = qDiagTitle;
    queueConfig.connectionUri = qConnectionUri;
    queueConfig.name = qName;
    queueConfig.exchange = qExchange;
    queueConfig.routingKey = qRoutingKey;
    return true;
}

bool ConfigLoader::parse(Config& destCfg)
{
    config_setting_t* consumeFrom;
    config_setting_t* publishTo;

    this->ossErr.clear();
    if ( access(this->configFile.c_str(), F_OK) == -1 ) {
        this->ossErr << "File is not found or not readable: '" << this->configFile.c_str() << "'";
        return false;
    }
    if ( !config_read_file(&this->cfg, this->configFile.c_str()) ) {
        this->ossErr << config_error_file(&this->cfg)
            << ":" << config_error_line(&this->cfg)
            << " " << config_error_text(&this->cfg);
        return false;
    }

    int enableDiag;
    if (!config_lookup_int(&this->cfg, "enableDiag", &enableDiag) ) {
        enableDiag = false;
    }

    if ( !config_lookup_string(&this->cfg, "connectionUri", &this->defaultConnectionUri) ) {
        this->defaultConnectionUri = "amqp://guest:guest@localhost:5672/";
    }

    consumeFrom = config_lookup(&this->cfg, "consumeFrom");
    if (!consumeFrom) {
        this->ossErr << "Config does not contains required attribute 'consumeFrom'";
        return false;
    }

    publishTo = config_lookup(&this->cfg, "publishTo");
    if (!publishTo) {
        this->ossErr << "Config does not contains required attribute 'publishTo'";
        return false;
    }

    std::vector<QueueConfig> consumers;

    int count = config_setting_length(consumeFrom);
    for (int i=0; i < count; i++) {
        config_setting_t* consumerQueue = config_setting_get_elem(consumeFrom, i);
        QueueConfig queueConfig;
        if (!this->parseQueue(consumerQueue, queueConfig)) {
            this->ossErr << ", see `consumeFrom[" << i << "]`";
            return false;
        }
        consumers.push_back(queueConfig);
    }

    QueueConfig queueConfig;
    if (!this->parseQueue(publishTo, queueConfig)) {
        this->ossErr << ", see `publishTo`";
        return false;
    }

    destCfg.enableDiag = enableDiag != 0;
    destCfg.consumers = std::move(consumers);
    destCfg.publisher = std::move(queueConfig);
    return true;
}
