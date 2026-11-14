#ifndef EVOLUTION_BASIC_LOG_CONFIG_H
#define EVOLUTION_BASIC_LOG_CONFIG_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "evolution/steps/logStep.h"

namespace Evolution {

    /**
     * \brief Settings of the classic log of an evolution of one population, building the corresponding LogModule.
     *
     * \code
     * Evolution::BasicLogConfig logConfig;
     * logConfig.population = "Population";
     * logConfig.trainingFitness = "trainingFitness";
     * std::shared_ptr<Evolution::LogModule> log = logConfig.buildModule();
     * module->addSubModule(log);
     * \endcode
     *
     * Columns: Gen, Pop_Size, Fit_Min, Fit_Avg, Fit_Max, then Val_Min, Val_Avg, Val_Max if a validation fitness is given, then T_Gen
     * (the time of the generation) and T_Total (the time since the start of the evolution).
     * The columns are steps of the log: add, remove or replace them by name to customize it.
     */
    struct BasicLogConfig {
        /// Stream receiving the log, the console by default. It must outlive the evolution.
        std::ostream* output = &std::cout;

        /// Separator of the values (e.g. ";" for a CSV file), empty to align them in columns of columnWidth characters.
        std::string separator = "";

        /// Width of the columns when there is no separator.
        size_t columnWidth = 14;

        /// Name of the log module.
        std::string name = "Log";

        /// Data: the individuals of the population.
        std::string population = "Population";

        /// Data: the training fitness of the population.
        std::string trainingFitness = "trainingFitness";

        /// Data: the validation fitness of the population, empty for no validation columns.
        std::string validationFitness = "";

        /// Default polymorphic destructor.
        virtual ~BasicLogConfig() = default;

        /**
         * \brief Build the log: the generation, the size of the population, its training fitness, its validation fitness if any, and the times.
         *
         * \return the log, named by name.
         * \throw std::runtime_error if the output is null.
         */
        virtual std::shared_ptr<LogModule> buildModule() const;

        /**
         * \brief Add to a log one column per step of a module, showing the duration in seconds of its last run (see Module::getLastDuration).
         *
         * The columns are named by the steps, a step holding steps (e.g. the validation) being one column. They are inserted before T_Gen
         * if the log has it, at the end otherwise. The log must come after the steps to show the current generation, e.g. in the module itself.
         *
         * \param[in,out] log the log receiving the columns.
         * \param[in] module the module whose steps are timed.
         */
        static void addStepTimeColumns(LogModule& log, const Module& module);

      protected:
        /**
         * \brief Return the fitness of the individuals of a population that have one.
         *
         * \param[in] state the data of the evolution.
         * \param[in] population name of the individuals of the population.
         * \param[in] fitness name of the fitness of the individuals.
         * \return the fitness values, empty if the fitness was not computed during the current generation.
         */
        static std::vector<double> fitnessOfPopulation(const EvolutionData& state, const std::string& population, const std::string& fitness);

        /**
         * \brief Return the current generation.
         *
         * \param[in] state the data of the evolution.
         */
        static double getGeneration(const EvolutionData& state);

        /**
         * \brief Return the time in seconds of the current generation: since the previous row, or since the start of the evolution for the generation 0.
         *
         * \param[in] state the data of the evolution.
         * \param[in,out] previousTime time of the evolution at the previous row, updated with the current one.
         */
        static double getGenerationTime(const EvolutionData& state, double& previousTime);

        /**
         * \brief Return the time in seconds since the start of the evolution (see EvolutionData::stopwatch).
         *
         * \param[in] state the data of the evolution.
         */
        static double getTotalTime(const EvolutionData& state);

        /**
         * \brief Return the duration in seconds of the last run of a step, NaN if the step does not exist anymore.
         *
         * \param[in] step the step, held weakly so that a log timing the module holding it does not keep it alive.
         */
        static double getStepTime(const std::weak_ptr<const Module>& step);

        /**
         * \brief Return the number of individuals of a population.
         *
         * \param[in] state the data of the evolution.
         * \param[in] population name of the individuals of the population.
         */
        static double getPopulationSize(const EvolutionData& state, const std::string& population);

        /**
         * \brief Return the minimum fitness of a population, NaN if the fitness was not computed during the current generation.
         *
         * \param[in] state the data of the evolution.
         * \param[in] population name of the individuals of the population.
         * \param[in] fitness name of the fitness of the individuals.
         */
        static double getMinFitness(const EvolutionData& state, const std::string& population, const std::string& fitness);

        /**
         * \brief Return the average fitness of a population, NaN if the fitness was not computed during the current generation.
         *
         * \param[in] state the data of the evolution.
         * \param[in] population name of the individuals of the population.
         * \param[in] fitness name of the fitness of the individuals.
         */
        static double getAvgFitness(const EvolutionData& state, const std::string& population, const std::string& fitness);

        /**
         * \brief Return the maximum fitness of a population, NaN if the fitness was not computed during the current generation.
         *
         * \param[in] state the data of the evolution.
         * \param[in] population name of the individuals of the population.
         * \param[in] fitness name of the fitness of the individuals.
         */
        static double getMaxFitness(const EvolutionData& state, const std::string& population, const std::string& fitness);
    };

}; // namespace Evolution

#endif // EVOLUTION_BASIC_LOG_CONFIG_H
