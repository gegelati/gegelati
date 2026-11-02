#ifndef EVOLUTION_EXPORT_STEP_H
#define EVOLUTION_EXPORT_STEP_H

#include <optional>
#include <string>
#include <utility>

#include "evolution/step.h"

namespace Evolution {

    /**
     * \brief Step exporting a list of individuals, or only its best individual, to a .dot file.
     */
    class ExportStep : public Step {
        protected:
            /// Name of the exported individuals.
            std::string individuals;

            /// Path of the file, without extension.
            std::string filePath;

            /// Indicates if a file is written for each generation.
            bool oneFilePerGeneration;

            /// Name of the fitness ranking the individuals to only export the best one, none to export them all.
            std::optional<std::string> bestFitness;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals name of the exported individuals, a list of individuals.
             * \param[in] filePath path of the file, without extension.
             * \param[in] oneFilePerGeneration indicates if a file is written for each generation (the generation number is added to the path) or if the file is overwritten.
             * \param[in] bestFitness name of a fitness to only export the best individual, none to export them all.
             * \param[in] name name of the step.
             */
            ExportStep(std::string individuals, std::string filePath, bool oneFilePerGeneration = false, std::optional<std::string> bestFitness = std::nullopt,
                       std::string name = "Export")
                : Step(std::move(name)), individuals{std::move(individuals)}, filePath{std::move(filePath)},
                  oneFilePerGeneration{oneFilePerGeneration}, bestFitness{std::move(bestFitness)} {}

            /// Inherited from Step
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Step
            virtual std::string details() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_EXPORT_STEP_H
