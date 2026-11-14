#include "evolution/module.h"

#include <algorithm>
#include <sstream>

std::string Evolution::Module::listNames(const std::set<std::string>& names)
{
    std::string result;
    for(const std::string& name: names) {
        result += (result.empty() ? "" : ", ") + name;
    }
    return result;
}

std::string Evolution::Module::label() const
{
    std::string text = this->details();
    if(this->period > 1) {
        text += (text.empty() ? "" : ", ") + std::string("every ") + std::to_string(this->period) + " generations";
    }
    return this->stepName + (text.empty() ? "" : " (" + text + ")");
}

std::string Evolution::Module::dataText() const
{
    std::string text;
    if(!this->requiredData().empty()) {
        text += "requires: " + listNames(this->requiredData());
    }
    if(!this->providedData().empty()) {
        text += std::string(text.empty() ? "" : " | ") + "provides: " + listNames(this->providedData());
    }
    return text;
}

Evolution::Module& Evolution::Module::setInitialization(std::shared_ptr<Module> initialization)
{
    if(initialization == nullptr) {
        initialization = std::make_shared<Module>(this->data, "Initialization");
    }
    this->data->mergeWith(*initialization->data);
    this->initialization = std::move(initialization);
    return *this->initialization;
}

void Evolution::Module::setPeriod(size_t period)
{
    if(period == 0) {
        throw std::runtime_error("Evolution::Module::setPeriod: the period must be positive");
    }
    this->period = period;
}

std::shared_ptr<Evolution::Module> Evolution::Module::findStep(const std::string& name) const
{
    for(const std::shared_ptr<Module>& step: this->steps) {
        if(step->name() == name) {
            return step;
        }
    }
    return nullptr;
}

std::shared_ptr<Evolution::Module> Evolution::Module::findRecursiveSubModule(const std::string& name) const
{
    if(std::shared_ptr<Module> found = this->findStep(name)) {
        return found;
    }
    for(const std::shared_ptr<Module>& step: this->steps) {
        if(std::shared_ptr<Module> found = step->findRecursiveSubModule(name)) {
            return found;
        }
    }
    return nullptr;
}

bool Evolution::Module::has(const std::string& name) const
{
    return this->findStep(name) != nullptr;
}

Evolution::EvolutionData& Evolution::Module::getData()
{
    return *this->data;
}

const Evolution::EvolutionData& Evolution::Module::getData() const
{
    return *this->data;
}

size_t Evolution::Module::indexOf(const std::string& name) const
{
    for(size_t idx = 0; idx < this->steps.size(); idx++) {
        if(this->steps.at(idx)->name() == name) {
            return idx;
        }
    }
    throw std::runtime_error("Evolution::Module: no step named '" + name + "' in the module '" + this->stepName + "'");
}

void Evolution::Module::checkAdded(const std::shared_ptr<Module>& step, const std::string& replaced) const
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Module: cannot add a null step to the module '" + this->stepName + "'");
    }
    if(step->name() != replaced && this->has(step->name())) {
        throw std::runtime_error("Evolution::Module: the module '" + this->stepName + "' already has a step named '" + step->name() + "'");
    }
}

Evolution::Module& Evolution::Module::addSubModule(std::shared_ptr<Module> step)
{
    this->checkAdded(step);
    this->data->mergeWith(*step->data);
    this->steps.push_back(std::move(step));
    return *this;
}

Evolution::Module& Evolution::Module::insertBefore(const std::string& position, std::shared_ptr<Module> step)
{
    this->checkAdded(step);
    const size_t index = this->indexOf(position);
    this->data->mergeWith(*step->data);
    this->steps.insert(this->steps.begin() + index, std::move(step));
    return *this;
}

Evolution::Module& Evolution::Module::insertAfter(const std::string& position, std::shared_ptr<Module> step)
{
    this->checkAdded(step);
    const size_t index = this->indexOf(position);
    this->data->mergeWith(*step->data);
    this->steps.insert(this->steps.begin() + index + 1, std::move(step));
    return *this;
}

Evolution::Module& Evolution::Module::replace(const std::string& replaced, std::shared_ptr<Module> step)
{
    this->checkAdded(step, replaced);
    const size_t index = this->indexOf(replaced);
    this->data->mergeWith(*step->data);
    this->steps.at(index) = std::move(step);
    return *this;
}

Evolution::Module& Evolution::Module::remove(const std::string& removed)
{
    this->steps.erase(this->steps.begin() + this->indexOf(removed));
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

void Evolution::Module::checkData(std::set<std::string>& available) const
{
    if(this->initialization != nullptr) {
        this->initialization->checkData(available);
    }

    if(!this->steps.empty()) {
        for(const std::shared_ptr<Module>& step: this->steps) {
            step->checkData(available);
        }
        return;
    }
    for(const std::string& name: this->requiredData()) {
        if(available.find(name) == available.end()) {
            throw std::runtime_error("Evolution::Module: step '" + this->stepName + "' requires " + name + ", which no previous step provides");
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
        const double start = state.stopwatch.elapsed();
        step->apply(state);
        step->lastDuration = state.stopwatch.elapsed() - start;
    }
}

void Evolution::Module::runInitialization(EvolutionData& state)
{
    if(this->initialization != nullptr) {
        this->initialization->apply(state);
    }
    this->initialized = true;
    for(const std::shared_ptr<Module>& step: this->steps) {
        step->runInitialization(state);
    }
}

void Evolution::Module::runGenerations(size_t nbGenerations)
{
    if(!this->initialized) {
        throw std::runtime_error("Evolution::Module::runGenerations: the evolution of the module '" + this->stepName + "' is not initialized, call evolve first");
    }
    EvolutionData& state = *this->data;

    // The generations are checked first, to fail before running anything.
    std::set<std::string> available = state.dataNames();
    this->checkData(available);

    for(size_t idx = 0; idx < nbGenerations; idx++) {
        state.generation = state.nbGenerationsDone + 1;
        this->apply(state);
        state.nbGenerationsDone++;
    }
}

Evolution::EvolutionData& Evolution::Module::evolve(size_t nbGenerations, uint64_t seed)
{
    EvolutionData& state = *this->data;

    // The initializations, then the generations, are checked first, to fail before changing anything.
    std::set<std::string> available = state.dataNames();
    this->checkData(available);

    // A new evolution: every initialization runs again, during the generation 0, and replaces the data of the previous evolution.
    state.generation = EvolutionData::initGeneration;
    state.nbGenerationsDone = 0;
    state.rng.setSeed(seed);
    state.stopwatch.reset();
    this->runInitialization(state);

    this->runGenerations(nbGenerations);
    return state;
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
            left += this->initialized ? " [done]" : " [pending]";
        }
        if(this->initialization->steps.empty()) {
            rows.emplace_back(left + ": " + this->initialization->label(), this->initialization->dataText());
        } else {
            rows.emplace_back(left, "");
            this->initialization->collectRows(rows, prefix + (last ? "   " : "|  "), data);
        }
    }
    for(const std::shared_ptr<Module>& step: this->steps) {
        const bool last = (++idxItem == nbItems);
        rows.emplace_back(prefix + (last ? "`- " : "|- ") + step->label(), step->steps.empty() ? step->dataText() : "");
        step->collectRows(rows, prefix + (last ? "   " : "|  "), data);
    }
}

std::string Evolution::Module::summary(const EvolutionData* data) const
{
    std::vector<std::pair<std::string, std::string>> rows;
    rows.emplace_back(this->label(), "");
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
