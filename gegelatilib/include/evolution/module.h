#ifndef EVOLUTION_MODULE_H
#define EVOLUTION_MODULE_H

#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "evolution/evolutionData.h"

namespace Evolution {

    /**
     * \brief Unit of an evolution: a stage (selection, mutation, evaluation, logging...), or a list of units.
     *
     * A module reads and updates the EvolutionData, identified by keys. A module is either a leaf, which
     * overrides apply (the steps such as MutateStep), or a container, which runs the modules it holds one after the other
     * (the default apply). Containers nest, so a population, a whole evolutionary algorithm or a set of algorithms
     * is a module. Evolving is running the root module generation after generation: evolve creates the data of
     * a new evolution and returns them, resume goes on with the data of an evolution.
     *
     * Each module can have an initialization, itself a module, run once on some data before the module first runs (see initialize).
     * The initialization of a container is therefore the concatenation of the initializations of its modules. Modules added
     * to a root that already evolved are initialized the next time it evolves, without touching the others.
     *
     * Modules are shared pointers so the same module can be used in several containers. Modules are edited through the
     * objects themselves: insertAfter(mutate, secondMutate). Editing and searching also look into the nested containers.
     * If the same module appears several times, its first occurrence is used.
     *
     * A module is identified by its address: its name is only a label for summaries and error messages,
     * and several modules may share it. A leaf holds no module.
     *
     * \code
     * auto module = std::make_shared<Module>("Evolution");
     * module->add(lgp.build());
     * module->add(std::make_shared<LambdaStep>("Link", link, required, provided));
     * module->add(tpg.build());
     * EvolutionData data = module->evolve(100);
     * module->resume(data, 50);
     * std::cout << module->summary(data);
     * \endcode
     */
    class Module {
        protected:
            /// Label of the step.
            std::string stepName;

            /// Number of generations between two runs of the step in a container.
            size_t period = 1;

            /// Steps held by the step, in execution order.
            std::vector<std::shared_ptr<Module>> steps;

            /// Module run once on a state before the step first runs, null if none.
            std::shared_ptr<Module> initialization = nullptr;

            /// Identifier of the step, used by the state to remember the steps it initialized.
            uint64_t stepId;

            /// Duration in seconds of the last run of the step by its container.
            double lastDuration = 0.0;

            /**
             * \brief Return the container holding a step, this one or a nested one.
             *
             * \param[in] step the step searched.
             * \param[out] index position of the step in the container returned.
             * \return the container, or null if the step is not in this step or the nested ones.
             */
            Module* ownerOf(const std::shared_ptr<Module>& step, size_t& index);

            /**
             * \brief Return the container holding a step, or throw.
             *
             * \param[in] step the step searched.
             * \param[out] index position of the step in the container returned.
             * \throw std::runtime_error if the step is not in the step.
             */
            Module& checkedOwnerOf(const std::shared_ptr<Module>& step, size_t& index);

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] name name of the step.
             */
            explicit Module(std::string name = "Steps");

            /// Default polymorphic destructor
            virtual ~Module() = default;

            // Disable copying to avoid accidental copies (use shared pointers instead).
            Module(const Module&) = delete;
            Module& operator=(const Module&) = delete;

            /// Steps can be moved, not copied.
            Module(Module&&) = default;

            /// Steps can be moved, not copied.
            Module& operator=(Module&&) = default;

            /// Return the name of the step.
            const std::string& name() const { return this->stepName; }

            /**
             * \brief Rename the step, e.g. to tell apart two steps of the same kind in a summary.
             *
             * \param[in] name the new name.
             */
            void setName(std::string name) { this->stepName = std::move(name); }

            /**
             * \brief Run the step only once every few generations (e.g. a costly evaluation).
             *
             * The step runs on generations 0, period, 2 * period... and always during the initialization.
             * Data written by the step are therefore older than the current generation most of the time
             * (see EvolutionData::isUpToDate).
             *
             * \param[in] period number of generations between two runs.
             * \throw std::runtime_error if the period is zero.
             */
            void setPeriod(size_t period);

            /// Return the number of generations between two runs of the step.
            size_t getPeriod() const { return this->period; }

            /**
             * \brief Return true if the step runs during a generation.
             *
             * \param[in] generation the generation, EvolutionData::initGeneration for the initialization.
             */
            bool runsAt(uint64_t generation) const
            {
                return generation == EvolutionData::initGeneration || generation % this->period == 0;
            }

            /**
             * \brief Return the duration in seconds of the last run of the step.
             *
             * It is measured by the container running the step (with a Stopwatch), so the step is the authority on its time,
             * not the state. The duration of a container includes the ones of its steps. It is zero until the step ran.
             */
            double getLastDuration() const { return this->lastDuration; }

            /**
             * \brief Set the step run once on a state before this step first runs.
             *
             * It typically creates the data the step needs, e.g. the first population. It runs
             * during the generation EvolutionData::initGeneration.
             *
             * \param[in] initialization the initialization, null for none.
             */
            void setInitialization(std::shared_ptr<Module> initialization) { this->initialization = std::move(initialization); }

            /// Return the initialization of the step, null if none.
            const std::shared_ptr<Module>& getInitialization() const { return this->initialization; }

            /**
             * \brief Add a step at the end of the steps.
             *
             * \param[in] step the added step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null.
             */
            Module& add(std::shared_ptr<Module> step);

            /**
             * \brief Insert a step just before another one.
             *
             * \param[in] position step before which the new one is inserted, in this step or a nested one.
             * \param[in] step the inserted step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null or the position is not in the step.
             */
            Module& insertBefore(const std::shared_ptr<Module>& position, std::shared_ptr<Module> step);

            /**
             * \brief Insert a step just after another one.
             *
             * \param[in] position step after which the new one is inserted, in this step or a nested one.
             * \param[in] step the inserted step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null or the position is not in the step.
             */
            Module& insertAfter(const std::shared_ptr<Module>& position, std::shared_ptr<Module> step);

            /**
             * \brief Replace a step by another one.
             *
             * \param[in] replaced the replaced step, in this step or a nested one.
             * \param[in] step the new step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null or the replaced step is not in the step.
             */
            Module& replace(const std::shared_ptr<Module>& replaced, std::shared_ptr<Module> step);

            /**
             * \brief Remove a step.
             *
             * \param[in] removed the removed step, in this step or a nested one.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is not in the step.
             */
            Module& remove(const std::shared_ptr<Module>& removed);

            /**
             * \brief Return the first step with a name, looking into the nested steps.
             *
             * \param[in] name name of the step.
             * \return the step, or null if no step has this name.
             */
            std::shared_ptr<Module> findStep(const std::string& name) const;

            /**
             * \brief Return the first step with a name, e.g. to edit the steps built by a preset.
             *
             * The nested steps are searched too.
             *
             * \tparam T type of the step.
             * \param[in] name name of the step.
             * \throw std::runtime_error if no step has this name, or if it is not a T.
             */
            template <typename T = Module> std::shared_ptr<T> find(const std::string& name) const
            {
                const std::shared_ptr<Module> step = this->findStep(name);
                if(step == nullptr) {
                    throw std::runtime_error("Evolution::Module::find: no step named '" + name + "'");
                }
                std::shared_ptr<T> found = std::dynamic_pointer_cast<T>(step);
                if(found == nullptr) {
                    throw std::runtime_error("Evolution::Module::find: the step '" + name + "' does not have the requested type");
                }
                return found;
            }

            /**
             * \brief Return true if a step has the name, in this step or a nested one.
             *
             * \param[in] name name of the step.
             */
            bool has(const std::string& name) const;

            /// Return the steps held by this step in execution order, nested steps being one step.
            const std::vector<std::shared_ptr<Module>>& getSteps() const { return this->steps; }

            /// Return the number of steps held by this step.
            size_t size() const { return this->steps.size(); }

            /**
             * \brief Return the names of the data of the state that must be filled before this step runs.
             *
             * For a container, the data required by a step and not provided by a previous one.
             */
            virtual std::set<std::string> requiredData() const;

            /// Return the names of the data of the state filled or updated by this step (for a container, by its steps).
            virtual std::set<std::string> providedData() const;

            /// Return an optional one line description of the parameters of the step, used in summaries.
            virtual std::string details() const { return ""; }

            /**
             * \brief Check that the step finds the data it requires filled by the previous steps.
             *
             * A container checks its steps one by one.
             *
             * \param[in,out] available names of the data already filled, updated with the data provided by the step.
             * \param[in] where location of the step used in the error message (e.g. "generation").
             * \throw std::runtime_error if the step requires data that no previous step provides.
             */
            virtual void checkData(std::set<std::string>& available, const std::string& where) const;

            /**
             * \brief Run the step.
             *
             * The default implementation is the one of a container: it runs the steps that run during the current
             * generation and times them (see getLastDuration). A leaf overrides it.
             *
             * \param[in,out] state the state of the evolution read and updated by the step.
             */
            virtual void apply(EvolutionData& state);

            /**
             * \brief Run the initializations that were not run on a state yet.
             *
             * The initialization of this step runs first if the state did not run it already, then the steps
             * are initialized in their order. Nothing happens for what is already initialized.
             *
             * \param[in,out] state the state of the evolution.
             */
            void initialize(EvolutionData& state);

            /**
             * \brief Start a new evolution: create its data, run generations, and return the data.
             *
             * The initializations run first, then the generations. See resume.
             *
             * \param[in] nbGenerations number of generations to run.
             * \param[in] seed seed of the random number generator of the evolution.
             * \return the data of the evolution.
             * \throw std::runtime_error if a module requires data that nothing provides.
             */
            EvolutionData evolve(size_t nbGenerations, uint64_t seed = 0);

            /**
             * \brief Go on with the evolution of some data, after initializing what is not initialized yet.
             *
             * The data the modules require are checked first: they must be in the data or provided by a previous
             * initialization or module. Calling resume again continues the same evolution, modules added meanwhile being
             * initialized. Use EvolutionData::reset to start again.
             *
             * \param[in,out] data the data of the evolution, e.g. returned by evolve.
             * \param[in] nbGenerations number of generations to run.
             * \throw std::runtime_error if a module requires data that nothing provides.
             */
            void resume(EvolutionData& data, size_t nbGenerations);

            /**
             * \brief Return a printable tree of the modules with their required and provided data.
             *
             * Each line shows a module with its parameters and period, and the leaves show the data they require
             * and provide in aligned columns. Initializations are shown under the module they belong to.
             */
            std::string summary() const;

            /**
             * \brief Return the tree of the modules (see summary()) then the given data, with the initializations done on them.
             *
             * \param[in] data the data of an evolution of this module.
             */
            std::string summary(const EvolutionData& data) const;

        private:
            /// Check the initializations that were not run on a state yet.
            void checkInitialization(const EvolutionData& state, std::set<std::string>& available) const;

            /// Return the tree of the modules, then the data if given.
            std::string summary(const EvolutionData* data) const;

            /// Add one row (text of the tree, data of the module) per module held by this one, initialization included.
            void collectRows(std::vector<std::pair<std::string, std::string>>& rows, const std::string& prefix, const EvolutionData* data) const;
    };

}; // namespace Evolution

#endif // EVOLUTION_MODULE_H
