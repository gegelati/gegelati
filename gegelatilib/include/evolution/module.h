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
     * is a module. The root module runs the evolution: evolve(nbGenerations, seed) starts a new one (the initializations, at the
     * generation 0, then the generations), runGenerations(nbGenerations) goes on with the current one.
     *
     * A module added to another one (addSubModule, insertBefore, setInitialization...) has its data merged into the data
     * of its new container (see EvolutionData::mergeWith), so that the root holds the data of the whole tree and runs the
     * evolution on them. The data hold the components of the evolution (problem, selectors, assigners...), which the steps
     * read by name, and the data produced by the steps (populations, metrics, fitness...).
     *
     * Each module can have an initialization, itself a module, run once per evolution before the module first runs (see evolve).
     * The initialization of a container is therefore the concatenation of the initializations of its modules. A module added
     * to a root that already evolved is initialized by the next evolve, which starts a new evolution: its initializations run again
     * and replace the data of the previous evolution.
     *
     * Modules are shared pointers. The modules held by a module are identified by their names, unique in it: they are edited and
     * searched by name (insertAfter("Mutation", secondMutate), find("Mutation")), in this module only, not in the nested ones.
     * findRecursiveSubModule searches the nested modules too. A leaf holds no module.
     *
     * \code
     * std::shared_ptr<Module> module = std::make_shared<Module>("Evolution");
     * module->addSubModule(lgp.buildModule());
     * module->addSubModule(std::make_shared<LambdaStep>("Link", link, required, provided));
     * module->addSubModule(tpg.buildModule());
     * module->evolve(100, seed);
     * module->runGenerations(50);
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

            /// Indicates if the initialization of the module (and of its modules) was run, see evolve.
            bool initialized = false;

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
                : stepName{std::move(name)}, data{std::move(data)} {}

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
             * \throw std::runtime_error if the step is null, or if the module already has a step with its name.
             */
            Module& addSubModule(std::shared_ptr<Module> step);

            /**
             * \brief Insert a step just before another one of this module.
             *
             * \param[in] position name of the step before which the new one is inserted.
             * \param[in] step the inserted step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null, if the module already has a step with its name, or if no step has the position name.
             */
            Module& insertBefore(const std::string& position, std::shared_ptr<Module> step);

            /**
             * \brief Insert a step just after another one of this module.
             *
             * \param[in] position name of the step after which the new one is inserted.
             * \param[in] step the inserted step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null, if the module already has a step with its name, or if no step has the position name.
             */
            Module& insertAfter(const std::string& position, std::shared_ptr<Module> step);

            /**
             * \brief Replace a step of this module by another one.
             *
             * \param[in] replaced name of the replaced step.
             * \param[in] step the new step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if the step is null, if another step has its name, or if no step has the replaced name.
             */
            Module& replace(const std::string& replaced, std::shared_ptr<Module> step);

            /**
             * \brief Remove a step of this module.
             *
             * \param[in] removed name of the removed step.
             * \return the step, to chain calls.
             * \throw std::runtime_error if no step has this name.
             */
            Module& remove(const std::string& removed);

            /**
             * \brief Return the step of this module with a name, the nested modules not searched.
             *
             * \param[in] name name of the step.
             * \return the step, or null if no step has this name.
             */
            std::shared_ptr<Module> findStep(const std::string& name) const;

            /**
             * \brief Return the first module with a name, in this module or in the nested ones.
             *
             * The steps of this module are searched first, then the nested modules in their order. Several nested modules may have
             * a step with the same name: prefer find on the module holding the step.
             *
             * \param[in] name name of the module.
             * \return the module, or null if no module has this name.
             */
            std::shared_ptr<Module> findRecursiveSubModule(const std::string& name) const;

            /**
             * \brief Return the step of this module with a name, e.g. to edit the steps built by a preset.
             *
             * The nested modules are not searched (see findRecursiveSubModule).
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
             * \brief Return true if a step of this module has the name, the nested modules not searched.
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
             * \throw std::runtime_error if the step requires data that no previous step provides.
             */
            virtual void checkData(std::set<std::string>& available) const;

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
             * \brief Start a new evolution and run it.
             *
             * The generation counters and the stopwatch of the data are reset, the seed is given to their random number generator,
             * then every initialization runs again (replacing the data of the previous evolution) before the generations.
             * The components, and the data of the previous evolution that no initialization replaces, stay in the data.
             *
             * \param[in] nbGenerations number of generations to run.
             * \param[in] seed seed of the random number generator of the evolution.
             * \return the data of the evolution (see getData).
             * \throw std::runtime_error if a module requires data that nothing provides.
             */
            EvolutionData& evolve(size_t nbGenerations, uint64_t seed = 0);

            /**
             * \brief Run more generations of the current evolution, started by evolve.
             *
             * No initialization runs: the evolution must have been started by evolve. A module added since then is only initialized by
             * the next evolve. The data required by the modules are checked before anything runs.
             *
             * \param[in] nbGenerations number of generations to run.
             * \throw std::runtime_error if the evolution was not started by evolve, or if a module requires data that nothing provides.
             */
            void runGenerations(size_t nbGenerations);

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
            /// Return the position of the step of this module with a name, or throw.
            size_t indexOf(const std::string& name) const;

            /// Check that a step can be added to this module (not null, name not taken), replacing the step with the given name if any.
            void checkAdded(const std::shared_ptr<Module>& step, const std::string& replaced = "") const;

            /// Run the initialization of the module, then the ones of its modules, without checking the data.
            void runInitialization(EvolutionData& state);

            /// Return the names of some data, separated by commas.
            static std::string listNames(const std::set<std::string>& names);

            /// Return the text of the module in the tree of a summary: its name, its parameters and its period.
            std::string label() const;

            /// Return the data required and provided by the module, written next to it in the tree of a summary.
            std::string dataText() const;

            /// Return the tree of the modules, then the data if given.
            std::string summary(const EvolutionData* data) const;

            /// Add one row (text of the tree, data of the module) per module held by this one, initialization included.
            void collectRows(std::vector<std::pair<std::string, std::string>>& rows, const std::string& prefix, const EvolutionData* data) const;
    };

}; // namespace Evolution

#endif // EVOLUTION_MODULE_H
