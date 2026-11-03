#include "evolution/steps/initPopulationStep.h"

#include <stdexcept>

Evolution::InitPopulationStep::InitPopulationStep(Key<std::vector<std::shared_ptr<Individual>>> individuals, std::shared_ptr<const Representations::Representation> representation, size_t size,
                                                  std::shared_ptr<const Mutation::Mutator> mutator, Key<GraphBased::GenotypeGenerator> generator, std::string name)
    : Module(std::move(name)), individuals{std::move(individuals)}, representation{std::move(representation)}, size{size}, mutator{std::move(mutator)},
      generator{std::move(generator)}
{
    if(this->representation == nullptr || this->mutator == nullptr) {
        throw std::runtime_error("Evolution::InitPopulationStep: the representation and the mutator cannot be null");
    }
}

std::set<std::string> Evolution::InitPopulationStep::requiredData() const
{
    return {this->generator.name()};
}

std::set<std::string> Evolution::InitPopulationStep::providedData() const
{
    return {this->individuals.name()};
}

std::string Evolution::InitPopulationStep::details() const
{
    return std::to_string(this->size) + " individuals";
}

void Evolution::InitPopulationStep::apply(EvolutionData& state)
{
    std::vector<std::shared_ptr<Individual>> individuals = Mutation::initIndividuals(*this->mutator, this->representation, *state.get(this->generator),
                                                                                     this->size, state.rng);
    state.set(this->individuals, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::move(individuals)));
}
