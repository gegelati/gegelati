#ifndef EVOLUTION_LAMBDA_STEP_H
#define EVOLUTION_LAMBDA_STEP_H

#include <functional>
#include <set>
#include <string>

#include "evolution/step.h"

namespace Evolution {

    /**
     * \brief Step running a user function, to add custom behaviors without writing a class.
     */
    class LambdaStep : public Step {
        protected:
            /// Function run by the step.
            std::function<void(EvolutionState&)> function;

            /// Data required by the function.
            std::set<std::string> required;

            /// Data filled or updated by the function.
            std::set<std::string> provided;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] name name of the step.
             * \param[in] function function run by the step.
             * \param[in] required data required by the function.
             * \param[in] provided data filled or updated by the function.
             */
            LambdaStep(std::string name, std::function<void(EvolutionState&)> function, std::set<std::string> required = {},
                       std::set<std::string> provided = {})
                : Step(std::move(name)), function{std::move(function)}, required{std::move(required)}, provided{std::move(provided)} {}

            /// Inherited from Step
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_LAMBDA_STEP_H
