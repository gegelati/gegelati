#include "evolution/basicLogConfig.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <vector>

#include "individual.h"

std::vector<double> Evolution::BasicLogConfig::fitnessOfPopulation(const EvolutionData& state, const std::string& population, const std::string& fitness)
{
    std::vector<double> values;
    if(state.isUpToDate(fitness)) {
        const std::shared_ptr<const std::map<uint64_t, double>> fitnessValues = state.get<std::map<uint64_t, double>>(fitness);
        const std::vector<std::shared_ptr<Individual>>& individuals = *state.get<std::vector<std::shared_ptr<Individual>>>(population);
        for(const std::shared_ptr<Individual>& individual: individuals) {
            const std::map<uint64_t, double>::const_iterator found = fitnessValues->find(individual->getIndividualID());
            if(found != fitnessValues->end()) {
                values.push_back(found->second);
            }
        }
    }
    return values;
}

double Evolution::BasicLogConfig::getGeneration(const EvolutionData& state)
{
    return static_cast<double>(state.generation);
}

double Evolution::BasicLogConfig::getGenerationTime(const EvolutionData& state, double& previousTime)
{
    // The generation 0 lasts since the start of the evolution, the others since the previous row.
    const double time = state.stopwatch.elapsed();
    const double duration = (state.generation == EvolutionData::initGeneration) ? time : time - previousTime;
    previousTime = time;
    return duration;
}

double Evolution::BasicLogConfig::getTotalTime(const EvolutionData& state)
{
    return state.stopwatch.elapsed();
}

double Evolution::BasicLogConfig::getStepTime(const std::weak_ptr<const Module>& step)
{
    const std::shared_ptr<const Module> timed = step.lock();
    return (timed != nullptr) ? timed->getLastDuration() : std::nan("");
}

void Evolution::BasicLogConfig::addStepTimeColumns(LogModule& log, const Module& module)
{
    for(const std::shared_ptr<Module>& step: module.getSteps()) {
        const std::weak_ptr<const Module> timed = step;
        std::shared_ptr<LogColumnStep> column = std::make_shared<LogColumnStep>(step->name(), [timed](const EvolutionData&) { return getStepTime(timed); }, 4);
        if(log.has("T_Gen")) {
            log.insertBefore("T_Gen", column);
        } else {
            log.addSubModule(column);
        }
    }
}

double Evolution::BasicLogConfig::getPopulationSize(const EvolutionData& state, const std::string& population)
{
    return static_cast<double>(state.get<std::vector<std::shared_ptr<Individual>>>(population)->size());
}

double Evolution::BasicLogConfig::getMinFitness(const EvolutionData& state, const std::string& population, const std::string& fitness)
{
    const std::vector<double> values = fitnessOfPopulation(state, population, fitness);
    return values.empty() ? std::nan("") : *std::min_element(values.begin(), values.end());
}

double Evolution::BasicLogConfig::getAvgFitness(const EvolutionData& state, const std::string& population, const std::string& fitness)
{
    const std::vector<double> values = fitnessOfPopulation(state, population, fitness);
    return values.empty() ? std::nan("") : std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
}

double Evolution::BasicLogConfig::getMaxFitness(const EvolutionData& state, const std::string& population, const std::string& fitness)
{
    const std::vector<double> values = fitnessOfPopulation(state, population, fitness);
    return values.empty() ? std::nan("") : *std::max_element(values.begin(), values.end());
}

std::shared_ptr<Evolution::LogModule> Evolution::BasicLogConfig::buildModule() const
{
    if(this->output == nullptr) {
        throw std::runtime_error("Evolution::BasicLogConfig::buildModule: the output is null");
    }

    // Step 1: create the log.
    std::shared_ptr<LogModule> log = std::make_shared<LogModule>(*this->output, this->separator, this->name);
    log->setColumnWidth(this->columnWidth);
    const std::string population = this->population;
    const std::string trainingFitness = this->trainingFitness;
    const std::string validationFitness = this->validationFitness;

    // Step 2: the generation.
    log->addSubModule(std::make_shared<LogColumnStep>("Gen", [](const EvolutionData& state) { return getGeneration(state); }, 0));

    // Step 3: the size of the population.
    log->addSubModule(std::make_shared<LogColumnStep>("Pop_Size",
        [population](const EvolutionData& state) { return getPopulationSize(state, population); }, 0, std::set<std::string>{population}));

    // Step 4: the minimum, the average and the maximum of the training fitness of the population.
    log->addSubModule(std::make_shared<LogColumnStep>("Fit_Min",
        [population, trainingFitness](const EvolutionData& state) { return getMinFitness(state, population, trainingFitness); }, 2,
        std::set<std::string>{population, trainingFitness}));
    log->addSubModule(std::make_shared<LogColumnStep>("Fit_Avg",
        [population, trainingFitness](const EvolutionData& state) { return getAvgFitness(state, population, trainingFitness); }, 2,
        std::set<std::string>{population, trainingFitness}));
    log->addSubModule(std::make_shared<LogColumnStep>("Fit_Max",
        [population, trainingFitness](const EvolutionData& state) { return getMaxFitness(state, population, trainingFitness); }, 2,
        std::set<std::string>{population, trainingFitness}));

    // Step 5: the same for the validation fitness, if any.
    if(!validationFitness.empty()) {
        log->addSubModule(std::make_shared<LogColumnStep>("Val_Min",
            [population, validationFitness](const EvolutionData& state) { return getMinFitness(state, population, validationFitness); }, 2,
            std::set<std::string>{population, validationFitness}));
        log->addSubModule(std::make_shared<LogColumnStep>("Val_Avg",
            [population, validationFitness](const EvolutionData& state) { return getAvgFitness(state, population, validationFitness); }, 2,
            std::set<std::string>{population, validationFitness}));
        log->addSubModule(std::make_shared<LogColumnStep>("Val_Max",
            [population, validationFitness](const EvolutionData& state) { return getMaxFitness(state, population, validationFitness); }, 2,
            std::set<std::string>{population, validationFitness}));
    }

    // Step 6: the time of the generation (since the previous row) and the time since the start of the evolution.
    const std::shared_ptr<double> previousTime = std::make_shared<double>(0.0);
    log->addSubModule(std::make_shared<LogColumnStep>("T_Gen", [previousTime](const EvolutionData& state) { return getGenerationTime(state, *previousTime); }, 2));
    log->addSubModule(std::make_shared<LogColumnStep>("T_Total", [](const EvolutionData& state) { return getTotalTime(state); }, 2));
    return log;
}
