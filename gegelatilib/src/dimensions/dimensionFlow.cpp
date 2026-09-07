
#include "dimensions/dimensionFlow.h"



bool Dimensions::DimensionFlow::addLayer(
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

bool Dimensions::DimensionFlow::isCompatibleWith(const Requirement& consumer) const noexcept {
    return hasOutput && currentOutput.isCompatibleWith(consumer);
}

bool Dimensions::DimensionFlow::isValid() const noexcept {
     return valid && hasOutput; 
}

const Dimensions::Requirement& Dimensions::DimensionFlow::getOutputDimension() const {
    if (!hasOutput) {
        throw std::logic_error("DimensionFlow has no output layer.");
    }
    return currentOutput;
}        

const std::vector<Dimensions::Requirement>& Dimensions::DimensionFlow::getInputDimensions() const {
    return inputDimensions;
}

std::string Dimensions::DimensionFlow::summary(const std::string& prefix) const {
    std::ostringstream result;

    // Header
    result << prefix << "=== Dimension Flow Summary ===\n\n";

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


bool Dimensions::DimensionFlow::acceptsRequirements(
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