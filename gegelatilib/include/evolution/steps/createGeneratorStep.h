#ifndef EVOLUTION_CREATE_GENERATOR_STEP_H
#define EVOLUTION_CREATE_GENERATOR_STEP_H

#include <functional>
#include <memory>
#include <set>
#include <string>

#include "evolution/module.h"
#include "graphBased/genotypeGenerator.h"
#include "representations/representation.h"

namespace Evolution {

    /**
     * \brief Step creating the generator of the genotypes and storing it in the data.
     *
     * The steps creating and mutating individuals read the generator in the data. Placed in the initialization, this step creates it
     * once. Placed in the generation too, it creates it again each time, e.g. from the current individuals of other populations.
     */
    class CreateGeneratorStep : public Module {
        protected:
            /// Key of the created generator.
            Key<GraphBased::GenotypeGenerator> generator;

            /// Function building the generator from the data.
            std::function<std::shared_ptr<GraphBased::GenotypeGenerator>(const EvolutionData&)> build;

            /// Names of the data read by the function.
            std::set<std::string> required;

        public:
            /**
             * \brief Constructor of the naive generator of a representation, sampling the values of its constraint.
             *
             * \param[in] generator key of the created generator.
             * \param[in] representation representation whose constraint is sampled, kept alive by the step.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the representation is null.
             */
            CreateGeneratorStep(Key<GraphBased::GenotypeGenerator> generator, std::shared_ptr<const Representations::Representation> representation,
                                std::string name = "CreateGenerator");

            /**
             * \brief Constructor of a generator built from the data.
             *
             * \param[in] generator key of the created generator.
             * \param[in] build function building the generator from the data, called each time the step runs.
             * \param[in] required names of the data read by the function, checked before the evolution runs.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the function is empty.
             */
            CreateGeneratorStep(Key<GraphBased::GenotypeGenerator> generator,
                                std::function<std::shared_ptr<GraphBased::GenotypeGenerator>(const EvolutionData&)> build,
                                std::set<std::string> required = {}, std::string name = "CreateGenerator");

            /// Inherited from Module
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Module
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_CREATE_GENERATOR_STEP_H
