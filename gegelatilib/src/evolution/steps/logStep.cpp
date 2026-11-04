#include "evolution/steps/logStep.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "evolution/steps/lambdaStep.h"

namespace {
    // Fitness of the individuals that have one, empty if it was not computed during the current generation.
    std::vector<double> fitnessOf(const Evolution::EvolutionData& state, const Evolution::Key<std::vector<std::shared_ptr<Individual>>>& individuals,
                                  const Evolution::Key<std::map<uint64_t, double>>& fitness)
    {
        std::vector<double> values;
        if(state.isUpToDate(fitness)) {
            for(const std::shared_ptr<Individual>& individual: *state.get(individuals)) {
                const auto found = state.get(fitness)->find(individual->getIndividualID());
                if(found != state.get(fitness)->end()) {
                    values.push_back(found->second);
                }
            }
        }
        return values;
    }

    // Text of a value, "-" if it is not a number.
    std::string format(double value, int precision)
    {
        if(std::isnan(value)) {
            return "-";
        }
        std::ostringstream text;
        text << std::fixed << std::setprecision(precision) << value;
        return text.str();
    }
}

Evolution::LogStep::LogStep(std::ostream& output, std::vector<Column> columns, std::string separator, std::string name)
    : Module(std::move(name)), output{&output}, columns{std::move(columns)}, separator{std::move(separator)}, stopwatch{std::make_shared<Stopwatch>()}
{
    // The times of the log start with the initialization of the step, whenever the evolution starts again.
    const std::shared_ptr<Stopwatch> started = this->stopwatch;
    this->initialization = std::make_shared<LambdaStep>("StartStopwatch", [started](EvolutionData&) { started->reset(); });
}

void Evolution::LogStep::writeLine(const std::vector<std::string>& texts)
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

void Evolution::LogStep::apply(EvolutionData& state)
{
    if(state.generation == EvolutionData::initGeneration) {
        return;
    }
    if(!this->headerWritten || state.generation == 0) {
        std::vector<std::string> headings{"Gen"};
        for(const Column& column: this->columns) {
            headings.push_back(column.name);
        }
        headings.insert(headings.end(), {"T_Gen", "T_Total"});
        this->writeLine(headings);
        this->headerWritten = true;
    }

    std::vector<std::string> values{std::to_string(state.generation)};
    for(const Column& column: this->columns) {
        values.push_back(format(column.value(state), column.precision));
    }
    this->stopwatch->checkpoint();
    values.push_back(format(this->stopwatch->getCheckpoints().back(), 2));
    values.push_back(format(this->stopwatch->elapsed(), 2));
    this->writeLine(values);
}

Evolution::LogStep::Column Evolution::sizeColumn(const Key<std::vector<std::shared_ptr<Individual>>>& individuals, std::string name)
{
    return {std::move(name), [individuals](const EvolutionData& state) { return static_cast<double>(state.has(individuals) ? state.get(individuals)->size() : 0); }, 0};
}

std::vector<Evolution::LogStep::Column> Evolution::fitnessColumns(const std::string& prefix, const Key<std::vector<std::shared_ptr<Individual>>>& individuals,
                                                                  const Key<std::map<uint64_t, double>>& fitness)
{
    return {{prefix + "_Min", [individuals, fitness](const EvolutionData& state) {
                const std::vector<double> values = fitnessOf(state, individuals, fitness);
                return values.empty() ? std::nan("") : *std::min_element(values.begin(), values.end());
            }},
            {prefix + "_Avg", [individuals, fitness](const EvolutionData& state) {
                const std::vector<double> values = fitnessOf(state, individuals, fitness);
                return values.empty() ? std::nan("") : std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
            }},
            {prefix + "_Max", [individuals, fitness](const EvolutionData& state) {
                const std::vector<double> values = fitnessOf(state, individuals, fitness);
                return values.empty() ? std::nan("") : *std::max_element(values.begin(), values.end());
            }}};
}

std::vector<Evolution::LogStep::Column> Evolution::stepTimeColumns(const Module& module)
{
    std::vector<LogStep::Column> columns;
    for(const std::shared_ptr<Module>& step: module.getSteps()) {
        if(step->size() == 0) {
            columns.push_back({step->name(), [step](const EvolutionData&) { return step->getLastDuration(); }, 4});
        } else {
            const std::vector<LogStep::Column> nested = stepTimeColumns(*step);
            columns.insert(columns.end(), nested.begin(), nested.end());
        }
    }
    return columns;
}
