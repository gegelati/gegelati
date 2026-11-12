#include "evolution/steps/initPopulationStep.h"

#include <stdexcept>

Evolution::InitPopulationStep::InitPopulationStep(std::string individuals, std::shared_ptr<const Representations::Representation> representation, size_t size,
                                                  std::shared_ptr<const Mutation::Mutator> mutator, std::string generator, std::string name)
    : Module(std::move(name)), individuals{std::move(individuals)}, representation{std::move(representation)}, size{size}, mutator{std::move(mutator)},
      generator{std::move(generator)}
{
    this->require(this->generator);
    this->provide(this->individuals);
    if(this->representation == nullptr || this->mutator == nullptr) {
        throw std::runtime_error("Evolution::InitPopulationStep: the representation and the mutator cannot be null");
    }
}

std::string Evolution::InitPopulationStep::details() const
{
    return std::to_string(this->size) + " individuals, " + this->representation->getRepresentationName() + ", " + this->mutator->getMutatorName();
}

void Evolution::InitPopulationStep::apply(EvolutionData& state)
{
    std::vector<std::shared_ptr<Individual>> individuals = Mutation::initIndividuals(*this->mutator, this->representation, *state.get<GraphBased::GenotypeGenerator>(this->generator),
                                                                                     this->size, state.rng);
    state.add(this->individuals, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::move(individuals)));
}
