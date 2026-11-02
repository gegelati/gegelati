#include "evolution/steps/mutateStep.h"

#include <stdexcept>

Evolution::MutateStep::MutateStep(std::string individuals, std::shared_ptr<const Mutation::Mutator> mutator,
                                  std::shared_ptr<const GraphBased::GenotypeGenerator> generator, size_t nbThreads, std::string name)
    : Step(std::move(name)), individuals{std::move(individuals)}, mutator{std::move(mutator)}, generator{std::move(generator)}, nbThreads{nbThreads}
{
    if(this->mutator == nullptr || this->generator == nullptr) {
        throw std::runtime_error("Evolution::MutateStep: the mutator and the generator cannot be null");
    }
}

std::set<std::string> Evolution::MutateStep::requiredData() const
{
    return {this->individuals};
}

std::string Evolution::MutateStep::details() const
{
    return this->individuals + ", " + std::to_string(this->nbThreads) + " thread(s)";
}

void Evolution::MutateStep::apply(EvolutionState& state)
{
    Mutation::mutateIndividualsInParallel(*this->mutator, *state.get<std::vector<std::shared_ptr<Individual>>>(this->individuals), *this->generator, state.rng, this->nbThreads);
}
