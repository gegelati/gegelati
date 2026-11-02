#include "evolution/pipeline.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace {
    std::string listNames(const std::set<std::string>& names)
    {
        std::string result;
        for(std::string name: names) {
            result += (result.empty() ? "" : ", ") + name;
        }
        return result;
    }
}

size_t Evolution::Pipeline::indexOf(const std::shared_ptr<Step>& step) const
{
    for(size_t idx = 0; idx < this->steps.size(); idx++) {
        if(this->steps.at(idx) == step) {
            return idx;
        }
    }
    throw std::runtime_error("Evolution::Pipeline: the step '" + (step == nullptr ? std::string("null") : step->name()) + "' is not in the pipeline");
}

Evolution::Pipeline& Evolution::Pipeline::add(std::shared_ptr<Step> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Pipeline::add: cannot add a null step");
    }
    this->steps.push_back(std::move(step));
    return *this;
}

Evolution::Pipeline& Evolution::Pipeline::insertBefore(const std::shared_ptr<Step>& position, std::shared_ptr<Step> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Pipeline::insertBefore: cannot insert a null step");
    }
    this->steps.insert(this->steps.begin() + this->indexOf(position), std::move(step));
    return *this;
}

Evolution::Pipeline& Evolution::Pipeline::insertAfter(const std::shared_ptr<Step>& position, std::shared_ptr<Step> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Pipeline::insertAfter: cannot insert a null step");
    }
    this->steps.insert(this->steps.begin() + this->indexOf(position) + 1, std::move(step));
    return *this;
}

Evolution::Pipeline& Evolution::Pipeline::replace(const std::shared_ptr<Step>& replaced, std::shared_ptr<Step> step)
{
    if(step == nullptr) {
        throw std::runtime_error("Evolution::Pipeline::replace: cannot replace by a null step");
    }
    this->steps.at(this->indexOf(replaced)) = std::move(step);
    return *this;
}

Evolution::Pipeline& Evolution::Pipeline::remove(const std::shared_ptr<Step>& removed)
{
    this->steps.erase(this->steps.begin() + this->indexOf(removed));
    return *this;
}

bool Evolution::Pipeline::has(const std::string& name) const
{
    return std::any_of(this->steps.begin(), this->steps.end(), [&name](const std::shared_ptr<Step>& step) { return step->name() == name; });
}

const std::vector<std::shared_ptr<Evolution::Step>>& Evolution::Pipeline::getSteps() const
{
    return this->steps;
}

size_t Evolution::Pipeline::size() const
{
    return this->steps.size();
}

void Evolution::Pipeline::checkData(std::set<std::string>& available, const std::string& pipelineName) const
{
    for(const std::shared_ptr<Step>& step: this->steps) {
        for(std::string name: step->requiredData()) {
            if(available.find(name) == available.end()) {
                throw std::runtime_error("Evolution::Pipeline: step '" + step->name() + "' of the " + pipelineName
                    + " pipeline requires " + name + ", which no previous step provides");
            }
        }
        for(std::string name: step->providedData()) {
            available.insert(name);
        }
    }
}

std::string Evolution::Pipeline::summary() const
{
    std::ostringstream result;
    for(size_t idx = 0; idx < this->steps.size(); idx++) {
        const Step& step = *this->steps.at(idx);
        result << "  " << (idx + 1) << ". " << step.name();
        std::string details = step.details();
        if(step.getPeriod() > 1) {
            details += (details.empty() ? "" : ", ") + std::string("every ") + std::to_string(step.getPeriod()) + " generations";
        }
        if(!details.empty()) {
            result << " (" << details << ")";
        }
        std::vector<std::pair<std::string, std::set<std::string>>> printedData;
        if(!step.requiredData().empty()) {
            printedData.push_back(std::make_pair("requires", step.requiredData()));
        }
        if(!step.providedData().empty()) {
            printedData.push_back(std::make_pair("provides", step.providedData()));
        }
        for(size_t idx = 0; idx < printedData.size(); idx++) {
            if(idx == 0) {
                result << "  [";
            }
            result << printedData.at(idx).first << ": " << listNames(printedData.at(idx).second);
            result << ((idx == printedData.size() - 1) ? "]" : " | ");
        }
        result << "\n";
    }
    return result.str();
}
