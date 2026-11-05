#include "evolution/module.h"

#include <algorithm>
#include <atomic>
#include <sstream>

#include "util/stopwatch.h"

namespace {
    std::string listNames(const std::set<std::string>& names)
    {
        std::string result;
        for(const std::string& name: names) {
            result += (result.empty() ? "" : ", ") + name;
        }
        return result;
    }

    // Text of a module in the tree: name, parameters, period and repetitions.
    std::string label(const Evolution::Module& module)
    {
        std::string details = module.details();
        if(module.getPeriod() > 1) {
            details += (details.empty() ? "" : ", ") + std::string("every ") + std::to_string(module.getPeriod()) + " generations";
        }
        return module.name() + (details.empty() ? "" : " (" + details + ")");
    }

    // Data required and provided by a leaf, written next to it in the tree.
    std::string dataText(const Evolution::Module& module)
    {
        std::string text;
        if(!module.requiredData().empty()) {
            text += "requires: " + listNames(module.requiredData());
        }
        if(!module.providedData().empty()) {
            text += std::string(text.empty() ? "" : " | ") + "provides: " + listNames(module.providedData());
        }
        return text;
    }
}

Evolution::Module::Module(std::string name) : stepName{std::move(name)}
{
    static std::atomic<uint64_t> nbSteps{0};
    this->stepId = nbSteps++;
}

void Evolution::Module::setPeriod(size_t period)
{
    if(period == 0) {
        throw std::runtime_error("Evolution::Module::setPeriod: the period must be positive");
    }
    this->period = period;
}

Evolution::Module* Evolution::Module::ownerOf(const std::shared_ptr<Module>& step, size_t& index)
{
    for(size_t idx = 0; idx < this->steps.size(); idx++) {
        if(this->steps.at(idx) == step) {
            index = idx;
            return this;
        }
    }
    for(const std::shared_ptr<Module>& nested: this->steps) {
        if(Module* owner = nested->ownerOf(step, index)) {
            return owner;
        }
    }
    return nullptr;
}

Evolution::Module& Evolution::Module::checkedOwnerOf(const std::shared_ptr<Module>& step, size_t& index)
{
    Module* owner = this->ownerOf(step, index);
    if(owner == nullptr) {
        throw std::runtime_error("Evolution::Module: the step '" + (step == nullptr ? std::string("null") : step->name()) + "' is not in the step '"
                                 + this->stepName + "'");
    }
    return *owner;
}

std::shared_ptr<Evolution::Module> Evolution::Module::findStep(const std::string& name) const
{
    for(const std::shared_ptr<Module>& step: this->steps) {
        if(step->name() == name) {
            return step;
        }
    }
    for(const std::shared_ptr<Module>& step: this->steps) {
        if(std::shared_ptr<Module> found = step->findStep(name)) {
            return found;
        }
    }
    return nullptr;
}

bool Evolution::Module::has(const std::string& name) const
{
    return this->findStep(name) != nullptr;
}

Evolution::Module& Evolution::Module::add(std::shared_ptr<Module> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Module::add: cannot add a null step");
    }
    this->steps.push_back(std::move(step));
    return *this;
}

Evolution::Module& Evolution::Module::insertBefore(const std::shared_ptr<Module>& position, std::shared_ptr<Module> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Module::insertBefore: cannot insert a null step");
    }
    size_t index = 0;
    Module& owner = this->checkedOwnerOf(position, index);
    owner.steps.insert(owner.steps.begin() + index, std::move(step));
    return *this;
}

Evolution::Module& Evolution::Module::insertAfter(const std::shared_ptr<Module>& position, std::shared_ptr<Module> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Module::insertAfter: cannot insert a null step");
    }
    size_t index = 0;
    Module& owner = this->checkedOwnerOf(position, index);
    owner.steps.insert(owner.steps.begin() + index + 1, std::move(step));
    return *this;
}

Evolution::Module& Evolution::Module::replace(const std::shared_ptr<Module>& replaced, std::shared_ptr<Module> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Module::replace: cannot replace by a null step");
    }
    size_t index = 0;
    Module& owner = this->checkedOwnerOf(replaced, index);
    owner.steps.at(index) = std::move(step);
    return *this;
}

Evolution::Module& Evolution::Module::remove(const std::shared_ptr<Module>& removed)
{
    size_t index = 0;
    Module& owner = this->checkedOwnerOf(removed, index);
    owner.steps.erase(owner.steps.begin() + index);
    return *this;
}

std::set<std::string> Evolution::Module::requiredData() const
{
    if(this->steps.empty()) {
        return this->requiredList;
    }
    std::set<std::string> required;
    std::set<std::string> provided;
    for(const std::shared_ptr<Module>& step: this->steps) {
        for(const std::string& name: step->requiredData()) {
            if(provided.find(name) == provided.end()) {
                required.insert(name);
            }
        }
        for(const std::string& name: step->providedData()) {
            provided.insert(name);
        }
    }
    return required;
}

std::set<std::string> Evolution::Module::providedData() const
{
    if(this->steps.empty()) {
        return this->providedList;
    }
    std::set<std::string> provided;
    for(const std::shared_ptr<Module>& step: this->steps) {
        for(const std::string& name: step->providedData()) {
            provided.insert(name);
        }
    }
    return provided;
}

void Evolution::Module::checkData(std::set<std::string>& available, const std::string& where) const
{
    if(!this->steps.empty()) {
        for(const std::shared_ptr<Module>& step: this->steps) {
            step->checkData(available, where);
        }
        return;
    }
    for(const std::string& name: this->requiredData()) {
        if(available.find(name) == available.end()) {
            throw std::runtime_error("Evolution::Module: step '" + this->stepName + "' of the " + where + " requires " + name
                                     + ", which no previous step provides");
        }
    }
    for(const std::string& name: this->providedData()) {
        available.insert(name);
    }
}

void Evolution::Module::apply(EvolutionData& state)
{
    for(const std::shared_ptr<Module>& step: this->steps) {
        if(!step->runsAt(state.generation)) {
            continue;
        }
        const Stopwatch stopwatch;
        step->apply(state);
        step->lastDuration = stopwatch.elapsed();
    }
}

void Evolution::Module::initialize(EvolutionData& state)
{
    if(state.initializedSteps.find(this->stepId) == state.initializedSteps.end()) {
        if(this->initialization != nullptr) {
            const uint64_t generation = state.generation;
            state.generation = EvolutionData::initGeneration;
            this->initialization->apply(state);
            state.generation = generation;
        }
        state.initializedSteps.insert(this->stepId);
    }
    for(const std::shared_ptr<Module>& step: this->steps) {
        step->initialize(state);
    }
}

void Evolution::Module::checkInitialization(const EvolutionData& state, std::set<std::string>& available) const
{
    if(this->initialization != nullptr && state.initializedSteps.find(this->stepId) == state.initializedSteps.end()) {
        this->initialization->checkData(available, "initialization of '" + this->stepName + "'");
    }
    for(const std::shared_ptr<Module>& step: this->steps) {
        step->checkInitialization(state, available);
    }
}

Evolution::EvolutionData Evolution::Module::evolve(size_t nbGenerations, uint64_t seed)
{
    EvolutionData data(seed);
    this->resume(data, nbGenerations);
    return data;
}

void Evolution::Module::resume(EvolutionData& state, size_t nbGenerations)
{
    std::set<std::string> available = state.dataNames();
    this->checkInitialization(state, available);
    this->checkData(available, "generations");

    this->initialize(state);
    for(size_t idx = 0; idx < nbGenerations; idx++) {
        state.generation = state.nbGenerationsDone + 1;
        this->apply(state);
        state.nbGenerationsDone++;
    }
}

void Evolution::Module::collectRows(std::vector<std::pair<std::string, std::string>>& rows, const std::string& prefix, const EvolutionData* data) const
{
    const size_t nbItems = this->steps.size() + ((this->initialization != nullptr) ? 1 : 0);
    size_t idxItem = 0;

    // The initialization comes first, then the modules in their order of execution.
    if(this->initialization != nullptr) {
        const bool last = (++idxItem == nbItems);
        std::string left = prefix + (last ? "`- " : "|- ") + "initialization";
        if(data != nullptr) {
            left += (data->initializedSteps.find(this->stepId) != data->initializedSteps.end()) ? " [done]" : " [pending]";
        }
        if(this->initialization->steps.empty()) {
            rows.emplace_back(left + ": " + label(*this->initialization), dataText(*this->initialization));
        } else {
            rows.emplace_back(left, "");
            this->initialization->collectRows(rows, prefix + (last ? "   " : "|  "), data);
        }
    }
    for(const std::shared_ptr<Module>& step: this->steps) {
        const bool last = (++idxItem == nbItems);
        rows.emplace_back(prefix + (last ? "`- " : "|- ") + label(*step), step->steps.empty() ? dataText(*step) : "");
        step->collectRows(rows, prefix + (last ? "   " : "|  "), data);
    }
}

std::string Evolution::Module::summary(const EvolutionData* data) const
{
    std::vector<std::pair<std::string, std::string>> rows;
    rows.emplace_back(label(*this), "");
    this->collectRows(rows, "", data);

    // Data of the leaves are aligned in a column.
    size_t width = 0;
    for(const auto& [left, right]: rows) {
        if(!right.empty()) {
            width = std::max(width, left.size());
        }
    }
    width = std::min<size_t>(width, 70);

    std::ostringstream result;
    for(const auto& [left, right]: rows) {
        result << left;
        if(!right.empty()) {
            result << std::string((left.size() < width) ? width - left.size() : 0, ' ') << "  " << right;
        }
        result << std::string(1, 10);
    }
    if(data != nullptr) {
        result << std::string(1, 10) << data->summary();
    }
    return result.str();
}

std::string Evolution::Module::summary() const
{
    return this->summary(nullptr);
}

std::string Evolution::Module::summary(const EvolutionData& data) const
{
    return this->summary(&data);
}
