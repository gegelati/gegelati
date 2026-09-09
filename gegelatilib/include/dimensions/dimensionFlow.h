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
         /**
          * \brief Describes a single transition (layer) in the execution pipeline.
          */
        struct Transition {
             /// \brief The name of the transition.
            std::string name;
             /// \brief The input requirements consumed by the transition.
            std::vector<Requirement> inputs;
             /// \brief The output requirement produced by the transition.
            Requirement output;
             /// \brief Whether the transition inputs are compatible with the available output.
            bool compatible;
         };

         /// \brief The input dimensions of the flow.
        std::vector<Requirement> inputDimensions;
         /// \brief The ordered transitions of the flow.
        std::vector<Transition> transitions;
         /// \brief The output requirement of the most recent transition.
        Requirement currentOutput;
         /// \brief Whether the flow has an output requirement.
        bool hasOutput = false;
         /// \brief Whether the flow is currently valid.
        bool valid = true;


      public:
         /**
          * \brief Constructs a DimensionFlow from the given input dimensions.
          *
          * \param inputDimensions The input dimensions of the flow.
          * \throw std::runtime_error If the input dimensions are empty or any
          *        input dimension has a null element type.
          */
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
          * \param name The name of the transition.
          * \param layerInputs The input requirements consumed by the transition.
          * \param output The output requirement produced by the transition.
          * \return True if the transition inputs are compatible with the current output.         
         */
        bool addLayer(
            std::string name,
            const std::vector<Requirement>& layerInputs,
            Requirement output);

          /**
           * \brief Checks whether the final flow output satisfies a consumer requirement.
           * \param consumer The consumer requirement to check against.
           * \return True if the flow output is compatible with the consumer requirement.
           */
        bool isCompatibleWith(const Requirement& consumer) const noexcept;

          /**
           * \brief Checks whether the flow is currently valid.
           * \return True if the flow is valid.
           */
        bool isValid() const noexcept;

          /**
           * \brief Returns the output requirement of the flow.
           * \return A reference to the current output requirement.
           */
        const Requirement& getOutputDimension() const;

          /**
           * \brief Returns the input dimensions of the flow.
           * \return A reference to the input dimensions.
           */
        const std::vector<Requirement>& getInputDimensions() const;

          /**
           * \brief Returns a readable summary of the execution pipeline.
           * \param prefix An optional prefix prepended to each summary line.
           * \return A string containing the pipeline summary.
           */
        std::string summary(const std::string& prefix = "") const;

          /**
           * \brief Checks whether the provided requirements satisfy the required requirements.
           * \param provided The provided requirements.
           * \param required The required requirements.
           * \return True if every required requirement is satisfied by a provided requirement.
           */
        static bool acceptsRequirements(
            const std::vector<Requirement>& provided,
            const std::vector<Requirement>& required);
    };

} // namespace Evolution

#endif // CONTROL_FLOW_H