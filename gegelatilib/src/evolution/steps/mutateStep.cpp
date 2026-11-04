#include "evolution/steps/mutateStep.h"

#include <stdexcept>

Evolution::MutateStep::MutateStep(std::string individuals, std::shared_ptr<const Mutation::Mutator> mutator, std::string generator,
                                  size_t nbThreads, std::string name)
    : Module(std::move(name)), individuals{std::move(individuals)}, mutator{std::move(mutator)}, generator{std::move(generator)}, nbThreads{nbThreads}
{
    this->require(this->individuals);
    this->require(this->generator);
    if(this->mutator == nullptr) {
        throw std::runtime_error("Evolution::MutateStep: the mutator cannot be null");
    }
}

std::string Evolution::MutateStep::details() const
{
    return this->individuals + ", " + std::to_string(this->nbThreads) + " thread(s)";
}

void Evolution::MutateStep::apply(EvolutionData& state)
{
    Mutation::mutateIndividualsInParallel(*this->mutator, *state.get<std::vector<std::shared_ptr<Individual>>>(this->individuals), *state.get<GraphBased::GenotypeGenerator>(this->generator), state.rng, this->nbThreads);
}
