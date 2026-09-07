#ifndef DIMENSION_FLOW_H
#define DIMENSION_FLOW_H

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "dimensions/requirement.h"

namespace Dimensions {

    /**
     * \brief Describes the data requirements through an ordered execution pipeline.
     */
    class DimensionFlow {
      private:
        struct Transition {
            std::string name;
            std::vector<Requirement> inputs;
            Requirement output;
            bool compatible;
        };

        std::vector<Requirement> inputDimensions;
        std::vector<Transition> transitions;
        Requirement currentOutput;
        bool hasOutput = false;
        bool valid = true;


      public:
        explicit DimensionFlow(std::vector<Requirement> inputDimensions)
            : inputDimensions(std::move(inputDimensions)) {
                if(this->inputDimensions.empty()) {
                    throw std::runtime_error("DimensionFlow::Constructor: Dimensions are invalid.");
                }
                for (const Requirement& input: this->inputDimensions) {
                    if (input.getDataType().elementType == nullptr) {
                        throw std::runtime_error("DimensionFlow::Constructor: Dimensions are invalid.");
                    } 
                }
            }

        /**
         * \brief Adds a transition and updates the current output requirement.
         *
         * The first transition consumes the flow inputs. Later transitions consume
         * the output of the preceding transition.
         */
        bool addLayer(
            std::string name,
            const std::vector<Requirement>& layerInputs,
            Requirement output);

        /** \brief Checks whether the final flow output satisfies a consumer requirement. */
        bool isCompatibleWith(const Requirement& consumer) const noexcept;

        bool isValid() const noexcept;

        const Requirement& getOutputDimension() const;

        const std::vector<Requirement>& getInputDimensions() const;

        /**
         * \brief Returns a readable summary of the execution pipeline.
         */
        std::string summary(const std::string& prefix = "") const;

        static bool acceptsRequirements(
            const std::vector<Requirement>& provided,
            const std::vector<Requirement>& required);
    };

} // namespace Evolution

#endif // CONTROL_FLOW_H