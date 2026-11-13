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
     * A module reads and updates the EvolutionData, identified by names. A module is either a leaf, which
     * overrides apply (the steps such as MutateStep), or a container, which runs the modules it holds one after the other
     * (the default apply). Containers nest, so a population, a whole evolutionary algorithm or a set of algorithms
     * is a module. An evolution has two stages on the root module: initialize seeds the data of the evolution
     * and runs the initializations (generation 0), evolve runs generations on initialized data.
     *
     * A module added to another one (addSubModule, insertBefore, setInitialization...) has its data merged into the data
     * of its new container (see EvolutionData::mergeWith), so that the root holds the data of the whole tree and runs the
     * evolution on them. The data hold the components of the evolution (problem, selectors, assigners...), which the steps
     * read by name, and the data produced by the steps (populations, metrics, fitness...).
     *
     * Each module can have an initialization, itself a module, run once on the data before the module first runs (see initialize).
     * The initialization of a container is therefore the concatenation of the initializations of its modules. A module added
     * to a root that already evolved is initialized alone by initializePending, evolve refusing data that are not initialized.
     *
     * Modules are shared pointers. Modules are edited through the
     * objects themselves: insertAfter(mutate, secondMutate). Editing and searching also look into the nested containers.
     * If the same module appears several times, its first occurrence is used.
     *
     * A module is identified by its address: its name is only a label for summaries and error messages,
     * and several modules may share it. A leaf holds no module.
     *
     * \code
     * std::shared_ptr<Module> module = std::make_shared<Module>("Evolution");
     * module->addSubModule(lgp.buildModule());
     * module->addSubModule(std::make_shared<LambdaStep>("Link", link, required, provided));
     * module->addSubModule(tpg.buildModule());
     * module->initialize(seed);
     * module->evolve(100);
     * module->evolve(50);
     * std::cout << module->summary(module->getData());
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

            /// Data accessible by the module.
            std::shared_ptr<Evolution::EvolutionData> data;

            /// Module run once on a state before the step first runs, null if none.
            std::shared_ptr<Module> initialization = nullptr;

            /// Identifier of the step, used by the state to remember the steps it initialized.
            uint64_t stepId;

            /// Names of the data required by the step itself (see require), not by the steps it holds.
            std::set<std::string> requiredList;

            /// Names of the data filled or updated by the step itself (see provide), not by the steps it holds.
            std::set<std::string> providedList;

            /// Declare a data that must be filled before the step runs. Meant to be called by the constructor of a step.
            void require(const std::string& name) { this->requiredList.insert(name); }

            /// Declare a data filled or updated by the step. Meant to be called by the constructor of a step.
            void provide(const std::string& name) { this->providedList.insert(name); }

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

            /// Return a new identifier for a step (defined in the library, so that every step of a program gets a different one).
            static uint64_t newStepId();

        public:
            /**
             * \brief Constructor, with its own empty data.
             *
             * \param[in] name name of the step.
             */
            Module(std::string name = "Steps") : Module(std::make_shared<EvolutionData>(), std::move(name)) {}

            /**
             * \brief Constructor.
             *
             * \param[in] data Data accessible by the module.
             * \param[in] name name of the step.
             */
            Module(std::shared_ptr<Evolution::EvolutionData> data, std::string name = "Steps")
                : stepName{std::move(name)}, data{std::move(data)}, stepId{newStepId()} {}

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
             * \brief Return the current data of the module
             */
            virtual EvolutionData& getData();

            /**
             * \brief Return the current data of the module
             */
            virtual const EvolutionData& getData() const;

            /**
             * \brief Run the step only once every few generations (e.g. a costly evaluation).
             *
             * The step runs on generations 0 (the initialization), period, 2 * period...
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
                return generation % this->period == 0;
            }

            /**
             * \brief Return the duration in seconds of the last run of the step.
             *
             * It is measured by the container running the step with the stopwatch of the data (EvolutionData::stopwatch). The duration of a container includes the ones of its steps. It is zero until the step ran.
             */
            double getLastDuration() const { return this->lastDuration; }

            /**
             * \brief Set the step run once on a state before this step first runs.
             *
             * It typically creates the data the step needs, e.g. the first population. It runs
             * during the generation EvolutionData::initGeneration. It shares the data of this module.
             *
             * \param[in] initialization the initialization, null to create an empty one (named "Initialization") to fill.
             * \return the initialization, to add its steps.
             */
            Evolution::Module& setInitialization(std::shared_ptr<Module> initialization = nullptr);

            /// Return the initialization of the step, null if none.
            const std::shared_ptr<Module>& getInitialization() const { return this->initialization; }

            /**
             * \brief Add a step at the end of the steps.
             *
             * \param[in] step the added step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null.
             */
            Module& addSubModule(std::shared_ptr<Module> step);

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
             * A step returns the names it declared with require. For a container, it is the data required by a step and not provided by a previous one.
             */
            std::set<std::string> requiredData() const;

            /// Return the names of the data of the state filled or updated by this step (for a container, by its steps).
            std::set<std::string> providedData() const;

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
             * \brief Seed the random number generator of the data and run the initializations not run yet (generation 0).
             *
             * The data are not reset, since they hold the components: to start a new evolution, build a new module.
             * The data required by the modules are checked first: they must be in the data (e.g. a component) or provided by a previous
             * initialization or module.
             *
             * \param[in] seed seed of the random number generator of the evolution.
             * \return the initialized data of the evolution (see getData).
             * \throw std::runtime_error if a module requires data that nothing provides.
             */
            EvolutionData& initialize(uint64_t seed = 0);

            /**
             * \brief Run the initializations that were not run yet on the data, without resetting them.
             *
             * The initialization of this module runs first if the data did not run it already, then the modules held
             * are initialized in their order. Nothing happens for what is already initialized, so calling it again only
             * initializes the modules added meanwhile.
             *
             * \throw std::runtime_error if a module requires data that nothing provides.
             */
            void initializePending();

            /// Return true if every initialization of the module and the modules it holds was run on its data.
            bool isInitialized() const;

            /**
             * \brief Run generations on the data, going on with the generations already done.
             *
             * The data required by the modules are checked first. Calling it again continues the same evolution.
             * Build a new module to start again.
             *
             * \param[in] nbGenerations number of generations to run.
             * \throw std::runtime_error if the data are not initialized (see initialize), or if a module requires data that nothing provides.
             */
            void evolve(size_t nbGenerations);

            /**
             * \brief Initialize the data then run generations: initialize then evolve.
             *
             * \param[in] nbGenerations number of generations to run.
             * \param[in] seed seed of the random number generator of the evolution.
             * \return the data of the evolution (see getData).
             * \throw std::runtime_error if a module requires data that nothing provides.
             */
            EvolutionData& evolve(size_t nbGenerations, uint64_t seed);

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
            /// Run the initializations that were not run on a state yet, without checking the data.
            void runInitializations(EvolutionData& state);

            /// Return the names of some data, separated by commas.
            static std::string listNames(const std::set<std::string>& names);

            /// Return the text of the module in the tree of a summary: its name, its parameters and its period.
            std::string label() const;

            /// Return the data required and provided by the module, written next to it in the tree of a summary.
            std::string dataText() const;

            /// Check the initializations that were not run on a state yet.
            void checkInitialization(const EvolutionData& state, std::set<std::string>& available) const;

            /// Return true if every initialization of the module and the modules it holds was run on a state.
            bool isInitializedOn(const EvolutionData& state) const;

            /// Return the tree of the modules, then the data if given.
            std::string summary(const EvolutionData* data) const;

            /// Add one row (text of the tree, data of the module) per module held by this one, initialization included.
            void collectRows(std::vector<std::pair<std::string, std::string>>& rows, const std::string& prefix, const EvolutionData* data) const;
    };

}; // namespace Evolution

#endif // EVOLUTION_MODULE_H
