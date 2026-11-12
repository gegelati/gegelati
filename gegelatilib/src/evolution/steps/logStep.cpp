#include "evolution/steps/logStep.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "evolution/steps/lambdaStep.h"

// Fitness of the individuals that have one, empty if it was not computed during the current generation.
std::vector<double> Evolution::fitnessOf(const EvolutionData& state, const std::string& individuals,
                              const std::string& fitness)
{
    std::vector<double> values;
    if(state.isUpToDate(fitness)) {
        for(const std::shared_ptr<Individual>& individual: *state.get<std::vector<std::shared_ptr<Individual>>>(individuals)) {
            const auto found = state.get<std::map<uint64_t, double>>(fitness)->find(individual->getIndividualID());
            if(found != state.get<std::map<uint64_t, double>>(fitness)->end()) {
                values.push_back(found->second);
            }
        }
    }
    return values;
}

// Text of a value, "-" if it is not a number.
std::string Evolution::LogStep::format(double value, int precision)
{
    if(std::isnan(value)) {
        return "-";
    }
    std::ostringstream text;
    text << std::fixed << std::setprecision(precision) << value;
    return text.str();
}

Evolution::LogStep::LogStep(std::ostream& output, std::vector<Column> columns, std::string separator, std::string name)
    : Module(std::move(name)), output{&output}, columns{std::move(columns)}, separator{std::move(separator)}
{
    for(const Column& column: this->columns) {
        this->requiredList.insert(column.required.begin(), column.required.end());
    }

    // The initialization of the step writes the row of the initial state (the generations done so far, zero for a new evolution).
    this->initialization = std::make_shared<LambdaStep>("WriteInitialState", [this](EvolutionData& state) { this->writeRow(state, state.nbGenerationsDone); });
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

void Evolution::LogStep::writeRow(const EvolutionData& state, uint64_t generation)
{
    if(!this->headerWritten || generation == 0) {
        std::vector<std::string> headings{"Gen"};
        for(const Column& column: this->columns) {
            headings.push_back(column.name);
        }
        headings.insert(headings.end(), {"T_Gen", "T_Total"});
        this->writeLine(headings);
        this->headerWritten = true;
    }

    std::vector<std::string> values{std::to_string(generation)};
    for(const Column& column: this->columns) {
        // The data of a column may not exist yet in the initial state (e.g. the offspring).
        const bool available = std::all_of(column.required.begin(), column.required.end(), [&state](const std::string& name) { return state.has(name); });
        values.push_back(format(available ? column.value(state) : std::nan(""), column.precision));
    }
    const double time = state.stopwatch.elapsed();
    values.push_back(format(time - ((generation == 0) ? 0.0 : this->previousTime), 2));
    values.push_back(format(time, 2));
    this->previousTime = time;
    this->writeLine(values);
}

void Evolution::LogStep::apply(EvolutionData& state)
{
    this->writeRow(state, state.generation);
}

Evolution::LogStep::Column Evolution::sizeColumn(const std::string& individuals, std::string name)
{
    return {std::move(name), [individuals](const EvolutionData& state) { return static_cast<double>(state.get<std::vector<std::shared_ptr<Individual>>>(individuals)->size()); }, 0, {individuals}};
}

std::vector<Evolution::LogStep::Column> Evolution::fitnessColumns(const std::string& prefix, const std::string& individuals,
                                                                  const std::string& fitness)
{
    return {{prefix + "_Min", [individuals, fitness](const EvolutionData& state) {
                const std::vector<double> values = fitnessOf(state, individuals, fitness);
                return values.empty() ? std::nan("") : *std::min_element(values.begin(), values.end());
            }, 2, {individuals, fitness}},
            {prefix + "_Avg", [individuals, fitness](const EvolutionData& state) {
                const std::vector<double> values = fitnessOf(state, individuals, fitness);
                return values.empty() ? std::nan("") : std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
            }, 2, {individuals, fitness}},
            {prefix + "_Max", [individuals, fitness](const EvolutionData& state) {
                const std::vector<double> values = fitnessOf(state, individuals, fitness);
                return values.empty() ? std::nan("") : *std::max_element(values.begin(), values.end());
            }, 2, {individuals, fitness}}};
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
