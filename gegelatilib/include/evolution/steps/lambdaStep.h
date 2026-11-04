#ifndef EVOLUTION_LAMBDA_STEP_H
#define EVOLUTION_LAMBDA_STEP_H

#include <functional>
#include <set>
#include <string>

#include "evolution/module.h"

namespace Evolution {

    /**
     * \brief Step running a user function, to add custom behaviors without writing a class.
     */
    class LambdaStep : public Module {
        protected:
            /// Function run by the step.
            std::function<void(EvolutionData&)> function;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] name name of the step.
             * \param[in] function function run by the step.
             * \param[in] required names of the data required by the function.
             * \param[in] provided names of the data filled or updated by the function.
             */
            LambdaStep(std::string name, std::function<void(EvolutionData&)> function, std::set<std::string> required = {}, std::set<std::string> provided = {})
                : Module(std::move(name)), function{std::move(function)}
            {
                this->requiredList = std::move(required);
                this->providedList = std::move(provided);
            }

            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_LAMBDA_STEP_H
