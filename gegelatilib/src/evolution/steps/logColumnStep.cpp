#include "evolution/steps/logColumnStep.h"

#include <cmath>
#include <iomanip>
#include <sstream>

Evolution::LogColumnStep::LogColumnStep(std::string name, std::function<double(const EvolutionData&)> value, int precision, std::set<std::string> required)
    : Module(std::move(name)), value{std::move(value)}, precision{precision}
{
    for(const std::string& data: required) {
        this->require(data);
    }
}

std::string Evolution::LogColumnStep::format(double value, int precision)
{
    if(std::isnan(value)) {
        return "-";
    }
    std::ostringstream text;
    text << std::fixed << std::setprecision(precision) << value;
    return text.str();
}

void Evolution::LogColumnStep::apply(EvolutionData& state)
{
    // The data of a column may not exist yet in the row of the initial state (e.g. the offspring).
    bool available = true;
    for(const std::string& data: this->requiredList) {
        available = available && state.has(data);
    }
    this->lastValue = available ? this->value(state) : std::nan("");
    this->lastGeneration = state.generation;
    this->hasRun = true;
}

std::string Evolution::LogColumnStep::text(const EvolutionData& state) const
{
    // A column that did not run during the generation (see setPeriod) has no value.
    if(!this->hasRun || this->lastGeneration != state.generation) {
        return "-";
    }
    return format(this->lastValue, this->precision);
}
