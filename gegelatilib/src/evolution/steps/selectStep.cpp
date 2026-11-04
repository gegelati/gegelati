#include "evolution/steps/selectStep.h"

#include <stdexcept>

namespace {
    // Names of lists, separated by commas.
    std::string namesOf(const std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>& lists)
    {
        std::string names;
        for(const Evolution::Key<std::vector<std::shared_ptr<Individual>>>& list: lists) {
            names += (names.empty() ? "" : ", ") + list.name();
        }
        return names;
    }
}


Evolution::SelectStep::SelectStep(std::vector<Key<std::vector<std::shared_ptr<Individual>>>> candidates, Key<std::map<uint64_t, double>> fitness, std::shared_ptr<const Selection::Selector> selector, size_t nbSelected,
                                  Key<std::vector<std::shared_ptr<Individual>>> selected, bool distinct, std::string name)
    : Module(std::move(name)), candidates{std::move(candidates)}, fitness{std::move(fitness)}, selector{std::move(selector)}, nbSelected{nbSelected},
      selected{std::move(selected)}, distinct{distinct}
{
    this->require(this->fitness);
    for(const Key<std::vector<std::shared_ptr<Individual>>>& key: this->candidates) {
        this->require(key);
    }
    this->provide(this->selected);
    if(this->candidates.empty() || this->selector == nullptr) {
        throw std::runtime_error("Evolution::SelectStep: the candidates cannot be empty and the selector cannot be null");
    }
}

std::string Evolution::SelectStep::details() const
{
    return std::to_string(this->nbSelected) + (this->distinct ? " distinct" : "") + " from " + namesOf(this->candidates)
        + " to " + this->selected.name();
}

void Evolution::SelectStep::apply(EvolutionData& state)
{
    std::unordered_map<uint64_t, std::shared_ptr<Individual>> index;
    for(const Key<std::vector<std::shared_ptr<Individual>>>& candidate: this->candidates) {
        for(const std::shared_ptr<Individual>& individual: *state.get(candidate)) {
            index.emplace(individual->getIndividualID(), individual);
        }
    }

    // Only the fitness of the candidates is used.
    std::map<uint64_t, double> candidatesFitness;
    for(const auto& [id, value]: *state.get(this->fitness)) {
        if(index.count(id) > 0) {
            candidatesFitness.emplace_hint(candidatesFitness.end(), id, value);
        }
    }
    const std::vector<uint64_t> ids = this->selector->select(candidatesFitness, this->nbSelected, state.rng);

    auto individuals = std::make_shared<std::vector<std::shared_ptr<Individual>>>();
    if(this->distinct) {
        for(uint64_t id: std::set<uint64_t>(ids.begin(), ids.end())) {
            individuals->push_back(index.at(id));
        }
    } else {
        for(uint64_t id: ids) {
            individuals->push_back(index.at(id));
        }
    }
    state.set(this->selected, std::move(individuals));
}
