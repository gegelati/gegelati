#include "evolution/steps/selectStep.h"

#include <stdexcept>

namespace {
    // Individuals of several lists, one after the other.
    std::vector<std::shared_ptr<const Individual>> getAllIndividuals(const Evolution::EvolutionState& state, const std::vector<std::string>& names)
    {
        std::vector<std::shared_ptr<const Individual>> individuals;
        for(const std::string& name: names) {
            const std::vector<std::shared_ptr<const Individual>> list = state.getIndividuals(name);
            individuals.insert(individuals.end(), list.begin(), list.end());
        }
        return individuals;
    }

    // Names of lists, separated by commas.
    std::string namesOf(const std::vector<std::string>& lists)
    {
        std::string names;
        for(const std::string& list: lists) {
            names += (names.empty() ? "" : ", ") + list;
        }
        return names;
    }
}


Evolution::SelectStep::SelectStep(std::vector<std::string> candidates, std::string fitness, std::shared_ptr<const Selection::Selector> selector, size_t nbSelected,
                                  std::string selected, bool distinct, std::string name)
    : Step(std::move(name)), candidates{std::move(candidates)}, fitness{std::move(fitness)}, selector{std::move(selector)}, nbSelected{nbSelected},
      selected{std::move(selected)}, distinct{distinct}
{
    if(this->candidates.empty() || this->selector == nullptr) {
        throw std::runtime_error("Evolution::SelectStep: the candidates cannot be empty and the selector cannot be null");
    }
}

std::set<std::string> Evolution::SelectStep::requiredData() const
{
    std::set<std::string> required(this->candidates.begin(), this->candidates.end());
    required.insert(this->fitness);
    return required;
}

std::set<std::string> Evolution::SelectStep::providedData() const
{
    return {this->selected};
}

std::string Evolution::SelectStep::details() const
{
    return std::to_string(this->nbSelected) + (this->distinct ? " distinct" : "") + " from " + namesOf(this->candidates)
        + " to " + this->selected;
}

void Evolution::SelectStep::apply(EvolutionState& state)
{
    const std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index = makeIndex(getAllIndividuals(state, this->candidates));

    // Only the fitness of the candidates is used.
    std::map<uint64_t, double> candidatesFitness;
    for(const auto& [id, value]: *state.get<std::map<uint64_t, double>>(this->fitness)) {
        if(index.count(id) > 0) {
            candidatesFitness.emplace_hint(candidatesFitness.end(), id, value);
        }
    }
    const std::vector<uint64_t> ids = this->selector->select(candidatesFitness, this->nbSelected, state.rng);

    auto individuals = std::make_shared<std::vector<std::shared_ptr<const Individual>>>();
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
