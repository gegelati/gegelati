#include "evolution/steps/initPopulationStep.h"

#include <stdexcept>

Evolution::InitPopulationStep::InitPopulationStep(std::string individuals, std::string representation, size_t size,
                                                  std::string mutator, std::string generator, std::string name)
    : Module(std::move(name)), individuals{std::move(individuals)}, representation{std::move(representation)}, size{size}, mutator{std::move(mutator)},
      generator{std::move(generator)}
{
    this->require(this->representation);
    this->require(this->mutator);
    this->require(this->generator);
    this->provide(this->individuals);
}

std::string Evolution::InitPopulationStep::details() const
{
    return std::to_string(this->size) + " individuals, " + this->representation + ", " + this->mutator;
}

void Evolution::InitPopulationStep::apply(EvolutionData& state)
{
    std::vector<std::shared_ptr<Individual>> individuals = Mutation::initIndividuals(
        *state.get<Mutation::Mutator>(this->mutator), state.get<const Representations::Representation>(this->representation),
        *state.get<GraphBased::GenotypeGenerator>(this->generator), this->size, state.rng);
    state.set(this->individuals, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::move(individuals)));
}
