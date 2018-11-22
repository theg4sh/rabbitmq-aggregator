#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <sstream>
#include <string>
#include <vector>
#include <libconfig.h>
#include "config.hpp"

#include <iostream>


class ConfigLoader
{
private:
    std::string configFile;
    std::ostringstream ossErr;
    config_t cfg;
    const char* defaultConnectionUri;

    bool parseQueue(config_setting_t* cfgQueue, QueueConfig& queueConfig);
public:
    ConfigLoader(const std::string& configFile): configFile(configFile)
    {
        config_init(&this->cfg);
    }

    std::string getError() { return this->ossErr.str(); }

    bool parse(Config& destCfg);

    ~ConfigLoader() {
        config_destroy(&this->cfg);
    }
};

#endif
