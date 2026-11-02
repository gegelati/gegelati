#ifndef EVOLUTION_MODULE_H
#define EVOLUTION_MODULE_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "evolution/evolutionState.h"
#include "evolution/pipeline.h"
#include "evolution/population.h"
#include "util/stopwatch.h"

/**
 * \brief namespace for object responsible of the main evolution
 */
namespace Evolution {

    /**
     * \brief Evolution of one or several populations, defined by two pipelines of steps.
     *
     * The initialization pipeline runs once and the generation pipeline runs once per generation,
     * both on the same EvolutionState. The module only holds the populations, runs the steps, times them,
     * checks that they can work together, and handles stop conditions. It knows nothing about
     * the data exchanged by the steps.
     *
     * \code
     * Evolution::Module module(seed);
     * auto lgp = module.addPopulation(lgpRepresentation, 100, "LGP");
     * module.addInitStep(std::make_shared<InitPopulationStep>(lgp, mutator));
     * module.addStep(std::make_shared<SelectStep>(std::vector<std::string>{lgp->individuals()}, lgp->data("fitness"), selector, 100, lgp->data("parents")));
     * ...
     * module.evolve(100);
     * \endcode
     *
     * Presets add populations and steps to a module (see BasicEvolution).
     */
    class Module {
        protected:
            /// Populations evolved by the module, kept alive with their representation.
            std::vector<std::shared_ptr<const Population>> populations;

            /// State shared by the steps.
            EvolutionState evolutionState;

            /// Steps run once to initialize the evolution.
            Pipeline initPipeline;

            /// Steps run at each generation.
            Pipeline generationPipeline;

            /// Seed of the random number generator of the evolution.
            uint64_t seed;

            /// Indicates if the initialization pipeline was run.
            bool initialized = false;

            /// Conditions stopping the evolution when one of them is true after a generation.
            std::vector<std::function<bool(const EvolutionState&)>> stopConditions;

            /// Stopwatch measuring the time of the evolution.
            Stopwatch stopwatch;

            /**
             * \brief Run all the steps of a pipeline that run during the current generation, and time them.
             *
             * \param[in] pipeline the pipeline run.
             */
            void runPipeline(const Pipeline& pipeline);

            /**
             * \brief Check that every step finds the data it requires.
             *
             * \throw std::runtime_error if a step requires data that no previous step provides.
             */
            void checkPipelines() const;

            /**
             * \brief Run generations until the requested number is reached or a stop condition is true.
             *
             * \param[in] nbGenerations maximum number of generations done.
             */
            void runGenerations(size_t nbGenerations);

        public:
            /**
             * \brief Constructor of a module without population nor steps.
             *
             * \param[in] seed seed of the random number generator of the evolution.
             */
            explicit Module(uint64_t seed = 0) : seed{seed} {}

            /**
             * \brief Add a population to evolve.
             *
             * The population is only described: steps working on it are added afterwards, starting with
             * an InitPopulationStep creating its individuals.
             *
             * \param[in] representation representation of the individuals, kept alive by the module.
             * \param[in] size number of individuals of the population.
             * \param[in] name name of the population, used to name its data in the state.
             * \return the population, to give to the steps working on it.
             * \throw std::runtime_error if the representation is null, or if the name is empty or already used.
             */
            std::shared_ptr<const Population> addPopulation(std::shared_ptr<const Representations::Representation> representation, size_t size, std::string name);

            /// Return the populations of the module, in insertion order.
            const std::vector<std::shared_ptr<const Population>>& getPopulations() const { return this->populations; }

            /**
             * \brief Add a step at the end of the initialization pipeline.
             *
             * \param[in] step the added step.
             * \return the step, to link other steps to it.
             */
            template <typename T> std::shared_ptr<T> addInitStep(std::shared_ptr<T> step)
            {
                this->initPipeline.add(step);
                return step;
            }

            /**
             * \brief Add a step at the end of the generation pipeline.
             *
             * \param[in] step the added step.
             * \return the step, to link other steps to it.
             */
            template <typename T> std::shared_ptr<T> addStep(std::shared_ptr<T> step)
            {
                this->generationPipeline.add(step);
                return step;
            }

            /// Return the pipeline run once at the beginning of the evolution, to edit it.
            Pipeline& initSteps() { return this->initPipeline; }

            /// Return the pipeline run once at the beginning of the evolution.
            const Pipeline& initSteps() const { return this->initPipeline; }

            /// Return the pipeline run at each generation, to edit it.
            Pipeline& generationSteps() { return this->generationPipeline; }

            /// Return the pipeline run at each generation.
            const Pipeline& generationSteps() const { return this->generationPipeline; }

            /// Return the current state of the evolution (individuals, fitness... see EvolutionState::getIndividuals and bestIndividual).
            EvolutionState& getState() { return this->evolutionState; }

            /// Return the current state of the evolution.
            const EvolutionState& getState() const { return this->evolutionState; }

            /**
             * \brief Add a condition stopping the evolution.
             *
             * Conditions are checked after each generation.
             *
             * \param[in] condition function returning true when the evolution must stop.
             */
            void addStopCondition(std::function<bool(const EvolutionState&)> condition);

            /**
             * \brief Check the pipelines, initialize the evolution and run generations.
             *
             * \param[in] nbGenerations maximum number of generations done.
             * \throw std::runtime_error if a step requires data that no previous step provides.
             */
            void evolve(size_t nbGenerations);

            /**
             * \brief Continue the evolution from its current state.
             *
             * Contrary to evolve, nothing is initialized: generations go on after the last one done.
             *
             * \param[in] nbGenerations maximum number of generations done.
             * \throw std::runtime_error if the evolution was not initialized.
             */
            void resume(size_t nbGenerations);

            /// Return the populations and the pipelines of the module as a printable text.
            std::string summary() const;
    };

}; // namespace Evolution

#endif // EVOLUTION_MODULE_H
