
#include "evolution/module.h"
#include "log/logger.h"

#include <utility>

void Evolution::Module::addLogger(std::shared_ptr<Log::Logger> logger)
{
    this->loggers.push_back(logger);
}

const std::vector<std::shared_ptr<Log::Logger>>& Evolution::Module::getLoggers() const
{
    return this->loggers;
}

std::string Evolution::Module::summary() const
{
    return "Not implemented";
}

void Evolution::Module::evolve(size_t nbGenerations)
{
    this->stopwatch.reset();

    this->initialize();
    
    for(std::shared_ptr<Log::Logger>& logger: this->loggers) {
        logger->initializeModule();
    }

    this->doGenerations(nbGenerations);

    
    for(std::shared_ptr<Log::Logger>& logger: this->loggers) {
        logger->endModule();
    }
}

void Evolution::Module::doGenerations(size_t nbGenerations)
{
    for(size_t idx = 0; idx < nbGenerations; idx++) {
        this->stopwatch.checkpointReset();

        this->doGeneration(idx);

        for(std::shared_ptr<Log::Logger>& logger: this->loggers) {
            logger->endGeneration();
        }
    }
}