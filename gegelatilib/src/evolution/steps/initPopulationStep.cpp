#include "evolution/steps/initPopulationStep.h"

#include <stdexcept>

Evolution::InitPopulationStep::InitPopulationStep(std::shared_ptr<const Population> population, std::shared_ptr<const Mutation::Mutator> mutator,
                                                  std::shared_ptr<const GraphBased::GenotypeGenerator> generator, std::string name)
    : Step(std::move(name)), population{std::move(population)}, mutator{std::move(mutator)}, generator{std::move(generator)}
{
    if(this->population == nullptr || this->mutator == nullptr) {
        throw std::runtime_error("Evolution::InitPopulationStep: the population and the mutator cannot be null");
    }
    if(this->generator == nullptr) {
        this->generator = this->population->getRepresentation()->getGenotypeGenerator().cloneUniquePtr();
    }
}

std::set<std::string> Evolution::InitPopulationStep::providedData() const
{
    return {this->population->individuals()};
}

std::string Evolution::InitPopulationStep::details() const
{
    return std::to_string(this->population->getSize()) + " individuals";
}

void Evolution::InitPopulationStep::apply(EvolutionState& state)
{
    std::vector<std::shared_ptr<Individual>> individuals = Mutation::initIndividuals(*this->mutator, *this->population->getRepresentation(), *this->generator,
                                                                                     this->population->getSize(), state.rng);
    state.set(this->population->individuals(), std::make_shared<std::vector<std::shared_ptr<const Individual>>>(individuals.begin(), individuals.end()));
}
