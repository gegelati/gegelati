#include "evolution/steps/logStep.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <memory>
#include <numeric>

#include "evolution/steps/lambdaStep.h"

namespace {
    struct FitnessStats {
        double min = std::nan("");
        double average = std::nan("");
        double max = std::nan("");
    };

    // Statistics of a fitness of individuals, nan if no individual has a fitness.
    FitnessStats computeStats(const std::vector<std::shared_ptr<const Individual>>& individuals, const std::map<uint64_t, double>& fitness)
    {
        FitnessStats stats;
        size_t count = 0;
        double sum = 0.0;
        for(const std::shared_ptr<const Individual>& individual: individuals) {
            auto it = fitness.find(individual->getIndividualID());
            if(it == fitness.end()) {
                continue;
            }
            stats.min = (count == 0) ? it->second : std::min(stats.min, it->second);
            stats.max = (count == 0) ? it->second : std::max(stats.max, it->second);
            sum += it->second;
            count++;
        }
        if(count > 0) {
            stats.average = sum / static_cast<double>(count);
        }
        return stats;
    }
}


Evolution::LogStep::LogStep(std::ostream& output, Key<std::vector<std::shared_ptr<Individual>>> individuals, std::vector<std::pair<std::string, Key<std::map<uint64_t, double>>>> fitnesses, std::vector<Column> columns,
                            std::string name)
    : Module(std::move(name)), output{&output}, individuals{std::move(individuals)}, fitnesses{std::move(fitnesses)}, columns{std::move(columns)}, stopwatch{std::make_shared<Stopwatch>()}
{
    // The times of the table start with the initialization of the step, whenever the evolution starts again.
    const std::shared_ptr<Stopwatch> started = this->stopwatch;
    this->initialization = std::make_shared<LambdaStep>("StartStopwatch", [started](EvolutionData&) { started->reset(); });
    if(this->columns.empty()) {
        const Key<std::vector<std::shared_ptr<Individual>>> logged = this->individuals;
        this->columns.push_back(Column{"Pop_Size", [logged](const EvolutionData& state) { return state.getIndividuals(logged).size(); }});
    }
}

std::set<std::string> Evolution::LogStep::requiredData() const
{
    std::set<std::string> required{this->individuals.name()};
    for(const auto& [prefix, fitness]: this->fitnesses) {
        required.insert(fitness.name());
    }
    return required;
}

void Evolution::LogStep::writeHeader()
{
    *this->output << std::setprecision(2) << std::fixed << std::right;
    *this->output << std::setw(columnWidth) << "Gen";
    for(const Column& column: this->columns) {
        *this->output << std::setw(columnWidth) << column.name;
    }
    for(const auto& [prefix, fitness]: this->fitnesses) {
        *this->output << std::setw(columnWidth) << prefix + "_Min" << std::setw(columnWidth) << prefix + "_Avg" << std::setw(columnWidth) << prefix + "_Max";
    }
    *this->output << std::setw(columnWidth) << "T_Gen" << std::setw(columnWidth) << "T_Total" << std::endl;
    this->headerWritten = true;
}

void Evolution::LogStep::apply(EvolutionData& state)
{
    if(state.generation == EvolutionData::initGeneration) {
        return;
    }
    if(!this->headerWritten || state.generation == 0) {
        this->writeHeader();
    }

    *this->output << std::setw(columnWidth) << state.generation;
    for(const Column& column: this->columns) {
        *this->output << std::setw(columnWidth) << column.value(state);
    }

    const std::vector<std::shared_ptr<const Individual>> logged = state.getIndividuals(this->individuals);
    for(const auto& [prefix, fitness]: this->fitnesses) {
        if(state.isUpToDate(fitness)) {
            const FitnessStats stats = computeStats(logged, *state.get(fitness));
            *this->output << std::setw(columnWidth) << stats.min << std::setw(columnWidth) << stats.average << std::setw(columnWidth) << stats.max;
        } else {
            *this->output << std::setw(columnWidth) << "-" << std::setw(columnWidth) << "-" << std::setw(columnWidth) << "-";
        }
    }

    this->stopwatch->checkpoint();
    const double generationTime = this->stopwatch->getCheckpoints().back();
    *this->output << std::setw(columnWidth) << generationTime << std::setw(columnWidth) << this->stopwatch->elapsed() << std::endl;
}
