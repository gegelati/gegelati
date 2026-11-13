#include "evolution/steps/selectStep.h"

#include <stdexcept>


Evolution::SelectStep::SelectStep(std::vector<std::string> candidates, std::string fitness, std::string selector, size_t nbSelected, std::string selected, std::string name)
    : Module(std::move(name)), candidates{std::move(candidates)}, fitness{std::move(fitness)}, selector{std::move(selector)}, nbSelected{nbSelected},
      selected{std::move(selected)}
{
    this->require(this->fitness);
    this->require(this->selector);
    for(const std::string& key: this->candidates) {
        this->require(key);
    }
    this->provide(this->selected);
    if(this->candidates.empty()) {
        throw std::runtime_error("Evolution::SelectStep: the candidates cannot be empty");
    }
}

std::string Evolution::SelectStep::details() const
{
    std::string names;
    for(const std::string& name: this->candidates) {
        names += (names.empty() ? "" : ", ") + name;
    }
    return this->selector + ", " + std::to_string(this->nbSelected) + " from " + names
        + " to " + this->selected;
}

void Evolution::SelectStep::apply(EvolutionData& state)
{
    std::unordered_map<uint64_t, std::shared_ptr<Individual>> index;
    for(const std::string& candidate: this->candidates) {
        for(const std::shared_ptr<Individual>& individual: *state.get<std::vector<std::shared_ptr<Individual>>>(candidate)) {
            index.emplace(individual->getIndividualID(), individual);
        }
    }

    // Only the fitness of the candidates is used.
    std::map<uint64_t, double> candidatesFitness;
    for(const auto& [id, value]: *state.get<std::map<uint64_t, double>>(this->fitness)) {
        if(index.count(id) > 0) {
            candidatesFitness.insert(std::make_pair(id, value));
        }
    }

    const std::vector<uint64_t> ids = state.get<Selection::Selector>(this->selector)->select(candidatesFitness, this->nbSelected, state.rng);

    std::shared_ptr<std::vector<std::shared_ptr<Individual>>> individuals = std::make_shared<std::vector<std::shared_ptr<Individual>>>();
    for(uint64_t id: ids) {
        individuals->push_back(index.at(id));
    }
    state.set(this->selected, std::move(individuals));
}
