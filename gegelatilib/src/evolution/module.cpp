#include "evolution/module.h"

#include <chrono>
#include <stdexcept>

std::shared_ptr<const Evolution::Population> Evolution::Module::addPopulation(std::shared_ptr<const Representations::Representation> representation, size_t size, std::string name)
{
    for(const std::shared_ptr<const Population>& population: this->populations) {
        if(population->name() == name) {
            throw std::runtime_error("Evolution::Module::addPopulation: a population is already named '" + name + "'");
        }
    }
    this->populations.push_back(std::make_shared<const Population>(std::move(representation), size, std::move(name)));
    return this->populations.back();
}

void Evolution::Module::addStopCondition(std::function<bool(const EvolutionState&)> condition)
{
    this->stopConditions.push_back(std::move(condition));
}

void Evolution::Module::runPipeline(const Pipeline& pipeline)
{
    for(const std::shared_ptr<Step>& step: pipeline.getSteps()) {
        if(!step->runsAt(this->evolutionState.generation)) {
            continue;
        }
        this->evolutionState.elapsedSeconds = this->stopwatch.elapsed();

        const auto start = std::chrono::steady_clock::now();
        step->apply(this->evolutionState);
        const std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;

        this->evolutionState.stepTimes.emplace_back(step->name(), duration.count());
    }
    this->evolutionState.elapsedSeconds = this->stopwatch.elapsed();
}

void Evolution::Module::checkPipelines() const
{
    std::set<std::string> available;
    this->initPipeline.checkData(available, "initialization");
    this->generationPipeline.checkData(available, "generation");
}

std::string Evolution::Module::summary() const
{
    std::string populations = "Populations:\n";
    for(const std::shared_ptr<const Population>& population: this->populations) {
        populations += "  " + population->name() + " (" + std::to_string(population->getSize()) + " individuals)\n";
    }
    return populations + "Initialization steps:\n" + this->initPipeline.summary() + "Generation steps:\n" + this->generationPipeline.summary();
}

void Evolution::Module::evolve(size_t nbGenerations)
{
    this->initialized = false;
    this->checkPipelines();
    this->stopwatch.reset();
    this->evolutionState.reset(this->seed);
    this->runPipeline(this->initPipeline);
    this->initialized = true;

    this->runGenerations(nbGenerations);
}

void Evolution::Module::resume(size_t nbGenerations)
{
    if(!this->initialized) {
        throw std::runtime_error("Evolution::Module::resume: the evolution must be initialized first (see evolve)");
    }
    this->runGenerations(nbGenerations);
}

void Evolution::Module::runGenerations(size_t nbGenerations)
{
    for(size_t idx = 0; idx < nbGenerations; idx++) {
        this->stopwatch.checkpointReset();

        this->evolutionState.generation = this->evolutionState.nbGenerationsDone;
        this->evolutionState.stepTimes.clear();
        this->runPipeline(this->generationPipeline);
        this->evolutionState.nbGenerationsDone++;

        for(const std::function<bool(const EvolutionState&)>& condition: this->stopConditions) {
            if(condition(this->evolutionState)) {
                return;
            }
        }
    }
}
