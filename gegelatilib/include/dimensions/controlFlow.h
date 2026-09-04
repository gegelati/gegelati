#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

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
    class ControlFlow {
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
        explicit ControlFlow(std::vector<Requirement> inputDimensions)
            : inputDimensions(std::move(inputDimensions)) {
                if(this->inputDimensions.empty()) {
                    throw std::runtime_error("ControlFlow::Constructor: Dimensions are invalid.");
                }
                for (const Requirement& input: this->inputDimensions) {
                    if (input.getDataType().elementType == nullptr) {
                        throw std::runtime_error("ControlFlow::Constructor: Dimensions are invalid.");
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
            Requirement output) {
            const bool compatible = hasOutput
                ? acceptsRequirements({currentOutput}, layerInputs)
                : acceptsRequirements(inputDimensions, layerInputs);
            valid = valid && compatible;

            transitions.push_back({
                std::move(name),
                layerInputs,
                output,
                compatible});
            currentOutput = std::move(output);
            hasOutput = true;
            return compatible;
        }

        /** \brief Checks whether the final flow output satisfies a consumer requirement. */
        bool isCompatibleWith(const Requirement& consumer) const noexcept {
            return hasOutput && currentOutput.isCompatibleWith(consumer);
        }

        bool isValid() const noexcept { return valid; }

        const Requirement& getOutputDimension() const {
            if (!hasOutput) {
                throw std::logic_error("ControlFlow has no output layer.");
            }
            return currentOutput;
        }        
        const std::vector<Requirement>& getInputDimensions() const {
            return inputDimensions;
        }

        /**
         * \brief Returns a readable summary of the execution pipeline.
         */
        std::string summary(const std::string& prefix = "") const {
            std::ostringstream result;

            // Header
            result << prefix << "=== Control Flow Summary ===\n\n";

            // Input Dimensions section
            result << prefix << "Inputs (" << inputDimensions.size() << "):\n";
            for (const auto& input : inputDimensions) {
                result << prefix << "  * " << input.summary() << "\n";
            }
            if (!inputDimensions.empty()) {
                result << "\n";
            }

            // Execution Pipeline section
            result << prefix << "Pipeline (" << transitions.size() << " layers):\n";
            for (size_t i = 0; i < transitions.size(); ++i) {
                const auto& t = transitions[i];
                result << prefix << "  Layer " << (i + 1) << ": " << t.name << "\n";
                for(size_t idx = 0; idx < t.inputs.size(); idx++) {
                    result << prefix << "    Input (" << idx << "): " << t.inputs.at(idx).summary() << "\n";
                }
                result << prefix << "    Output: " << t.output.summary() << "\n";
                result << prefix << "    Compatible: " << (t.compatible ? "YES" : "NO");
                if (!t.compatible) {
                    result << " [INCOMPATIBLE INPUT]";
                }
                result << "\n";

                if (i < transitions.size() - 1) {
                    result << "\n";
                }
            }

            // Final status
            if (!transitions.empty()) {
                result << "\n";
            }
            result << prefix << "Overall: " << (valid ? "VALID" : "INVALID") << "\n";

            return result.str();
        }
        static bool acceptsRequirements(
            const std::vector<Requirement>& provided,
            const std::vector<Requirement>& required) {
            if (provided.size() != required.size()) {
                return false;
            }
            for (size_t index = 0; index < provided.size(); ++index) {
                if (!provided[index].isCompatibleWith(required[index])) {
                    return false;
                }
            }
            return true;
        }
    };

} // namespace Evolution

#endif // CONTROL_FLOW_H