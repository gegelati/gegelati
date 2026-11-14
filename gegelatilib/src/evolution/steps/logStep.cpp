#include "evolution/steps/logStep.h"

#include <iomanip>

#include "evolution/steps/lambdaStep.h"

Evolution::LogModule::LogModule(std::ostream& output, std::string separator, std::string name)
    : Module(std::move(name)), output{&output}, separator{std::move(separator)}
{
    // The initialization writes the header, then the row of the initial state.
    this->initialization = std::make_shared<LambdaStep>("WriteHeader", [this](EvolutionData& state) { this->writeInitialRows(state); });
}

void Evolution::LogModule::writeInitialRows(EvolutionData& state)
{
    this->writeHeader();
    this->apply(state);
}

void Evolution::LogModule::writeLine(const std::vector<std::string>& texts)
{
    for(size_t idx = 0; idx < texts.size(); idx++) {
        if(this->separator.empty()) {
            *this->output << std::right << std::setw(static_cast<int>(this->columnWidth)) << texts.at(idx);
        } else {
            *this->output << ((idx > 0) ? this->separator : "") << texts.at(idx);
        }
    }
    *this->output << std::endl;
}

void Evolution::LogModule::writeHeader()
{
    std::vector<std::string> headings;
    for(const std::shared_ptr<Module>& step: this->steps) {
        if(std::dynamic_pointer_cast<LogColumnStep>(step) != nullptr) {
            headings.push_back(step->name());
        }
    }
    this->writeLine(headings);
}

void Evolution::LogModule::writeRow(const EvolutionData& state)
{
    std::vector<std::string> values;
    for(const std::shared_ptr<Module>& step: this->steps) {
        if(const std::shared_ptr<LogColumnStep> column = std::dynamic_pointer_cast<LogColumnStep>(step)) {
            values.push_back(column->text(state));
        }
    }
    this->writeLine(values);
}

void Evolution::LogModule::apply(EvolutionData& state)
{
    // The columns compute their values, then the row is written.
    Module::apply(state);
    this->writeRow(state);
}
