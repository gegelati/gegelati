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
             * \param[in] required keys of the data required by the function (typed keys have their type checked).
             * \param[in] provided keys of the data filled or updated by the function.
             */
            LambdaStep(std::string name, std::function<void(EvolutionData&)> function, std::vector<KeyBase> required = {},
                       std::vector<KeyBase> provided = {})
                : Module(std::move(name)), function{std::move(function)}
            {
                this->requiredKeyList = std::move(required);
                this->providedKeyList = std::move(provided);
            }

            /**
             * \brief Constructor with untyped data, given by their names.
             *
             * \param[in] name name of the step.
             * \param[in] function function run by the step.
             * \param[in] required names of the data required by the function.
             * \param[in] provided names of the data filled or updated by the function.
             */
            LambdaStep(std::string name, std::function<void(EvolutionData&)> function, std::set<std::string> required, std::set<std::string> provided = {})
                : Module(std::move(name)), function{std::move(function)}
            {
                this->requiredKeyList.assign(required.begin(), required.end());
                this->providedKeyList.assign(provided.begin(), provided.end());
            }

            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_LAMBDA_STEP_H
