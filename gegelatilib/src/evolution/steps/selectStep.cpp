#include "evolution/steps/selectStep.h"

#include <stdexcept>


Evolution::SelectStep::SelectStep(std::vector<std::string> candidates, std::string fitness, std::shared_ptr<const Selection::Selector> selector, size_t nbSelected,
                                  std::string selected, bool distinct, std::string name)
    : Module(std::move(name)), candidates{std::move(candidates)}, fitness{std::move(fitness)}, selector{std::move(selector)}, nbSelected{nbSelected},
      selected{std::move(selected)}, distinct{distinct}
{
    this->require(this->fitness);
    for(const std::string& key: this->candidates) {
        this->require(key);
    }
    this->provide(this->selected);
    if(this->candidates.empty() || this->selector == nullptr) {
        throw std::runtime_error("Evolution::SelectStep: the candidates cannot be empty and the selector cannot be null");
    }
}

std::string Evolution::SelectStep::details() const
{
    std::string names;
    for(const std::string& name: this->candidates) {
        names += (names.empty() ? "" : ", ") + name;
    }
    return this->selector->getSelectorName() + ", " + std::to_string(this->nbSelected) + (this->distinct ? " distinct" : "") + " from " + names
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
