#ifndef EVOLUTION_PIPELINE_H
#define EVOLUTION_PIPELINE_H

#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "evolution/step.h"

namespace Evolution {

    /**
     * \brief Ordered list of steps.
     *
     * Steps are shared pointers so the same step can be used in several pipelines
     * (e.g. the evaluation of the initialization and of each generation).
     * Steps are edited through the step objects themselves: insertAfter(mutate, secondMutate).
     * If the same step appears several times in a pipeline, its first occurrence is used.
     */
    class Pipeline {
        protected:
            /// Steps of the pipeline, in execution order.
            std::vector<std::shared_ptr<Step>> steps;

            /**
             * \brief Return the index of the first occurrence of a step.
             *
             * \param[in] step the step searched.
             * \throw std::runtime_error if the step is not in the pipeline.
             */
            size_t indexOf(const std::shared_ptr<Step>& step) const;

        public:
            /**
             * \brief Add a step at the end of the pipeline.
             *
             * \param[in] step the added step.
             * \return the pipeline, to chain calls.
             * \throw std::runtime_error if the step is null.
             */
            Pipeline& add(std::shared_ptr<Step> step);

            /**
             * \brief Insert a step just before another one.
             *
             * \param[in] position step before which the new one is inserted.
             * \param[in] step the inserted step.
             * \return the pipeline, to chain calls.
             * \throw std::runtime_error if the step is null or the position is not in the pipeline.
             */
            Pipeline& insertBefore(const std::shared_ptr<Step>& position, std::shared_ptr<Step> step);

            /**
             * \brief Insert a step just after another one.
             *
             * \param[in] position step after which the new one is inserted.
             * \param[in] step the inserted step.
             * \return the pipeline, to chain calls.
             * \throw std::runtime_error if the step is null or the position is not in the pipeline.
             */
            Pipeline& insertAfter(const std::shared_ptr<Step>& position, std::shared_ptr<Step> step);

            /**
             * \brief Replace a step by another one.
             *
             * \param[in] replaced the replaced step.
             * \param[in] step the new step.
             * \return the pipeline, to chain calls.
             * \throw std::runtime_error if the step is null or the replaced step is not in the pipeline.
             */
            Pipeline& replace(const std::shared_ptr<Step>& replaced, std::shared_ptr<Step> step);

            /**
             * \brief Remove a step.
             *
             * \param[in] removed the removed step.
             * \return the pipeline, to chain calls.
             * \throw std::runtime_error if the step is not in the pipeline.
             */
            Pipeline& remove(const std::shared_ptr<Step>& removed);

            /**
             * \brief Return the first step with a name, e.g. to edit a pipeline built by a preset.
             *
             * \tparam T type of the step.
             * \param[in] name name of the step.
             * \throw std::runtime_error if no step has this name, or if it is not a T.
             */
            template <typename T = Step> std::shared_ptr<T> find(const std::string& name) const
            {
                for(const std::shared_ptr<Step>& step: this->steps) {
                    if(step->name() == name) {
                        std::shared_ptr<T> found = std::dynamic_pointer_cast<T>(step);
                        if(found == nullptr) {
                            throw std::runtime_error("Evolution::Pipeline::find: the step '" + name + "' does not have the requested type");
                        }
                        return found;
                    }
                }
                throw std::runtime_error("Evolution::Pipeline::find: no step named '" + name + "'");
            }

            /**
             * \brief Return true if a step has the name.
             *
             * \param[in] name name of the step.
             */
            bool has(const std::string& name) const;

            /// Return the steps in execution order.
            const std::vector<std::shared_ptr<Step>>& getSteps() const;

            /// Return the number of steps.
            size_t size() const;

            /**
             * \brief Check that every step finds the data it requires filled by the previous steps.
             *
             * \param[in,out] available data already filled, updated with the data provided by the steps.
             * \param[in] pipelineName name of the pipeline used in the error message.
             * \throw std::runtime_error if a step requires data that no previous step provides.
             */
            void checkData(std::set<std::string>& available, const std::string& pipelineName) const;

            /// Return a printable list of the steps with their required and provided data.
            std::string summary() const;
    };

}; // namespace Evolution

#endif // EVOLUTION_PIPELINE_H
