
#include "dimensions/dimensionFlow.h"



bool Dimensions::DimensionFlow::addLayer(
    std::string name,
    const std::vector<Requirement>& layerInputs,
    Requirement output) {
    const bool compatible = this->hasOutput
        ? acceptsRequirements({this->currentOutput}, layerInputs)
        : acceptsRequirements(this->inputDimensions, layerInputs);
    this->valid = this->valid && compatible;

    this->transitions.push_back({
        std::move(name),
        layerInputs,
        output,
        compatible});
    this->currentOutput = std::move(output);
    this->hasOutput = true;
    return compatible;
}

bool Dimensions::DimensionFlow::isCompatibleWith(const Requirement& consumer) const noexcept {
    return this->hasOutput && this->currentOutput.isCompatibleWith(consumer);
}

bool Dimensions::DimensionFlow::isValid() const noexcept {
     return this->valid && this->hasOutput; 
}

const Dimensions::Requirement& Dimensions::DimensionFlow::getOutputDimension() const {
    if (!this->hasOutput) {
        throw std::logic_error("DimensionFlow has no output layer.");
    }
    return this->currentOutput;
}        

const std::vector<Dimensions::Requirement>& Dimensions::DimensionFlow::getInputDimensions() const {
    return this->inputDimensions;
}

std::string Dimensions::DimensionFlow::summary(const std::string& prefix) const {
    std::ostringstream result;

    // Header
    result << prefix << "=== Dimension Flow Summary ===\n\n";

    // Input Dimensions section
    result << prefix << "Inputs (" << this->inputDimensions.size() << "):\n";
    for (const auto& input : this->inputDimensions) {
        result << prefix << "  * " << input.summary() << "\n";
    }
    if (!this->inputDimensions.empty()) {
        result << "\n";
    }

    // Execution Pipeline section
    result << prefix << "Pipeline (" << this->transitions.size() << " layers):\n";
    for (size_t i = 0; i < this->transitions.size(); ++i) {
        const auto& transition = this->transitions[i];
        result << prefix << "  Layer " << (i + 1) << ": " << transition.name << "\n";
        for(size_t idx = 0; idx < transition.inputs.size(); idx++) {
            result << prefix << "    Input (" << idx << "): " << transition.inputs.at(idx).summary() << "\n";
        }
        result << prefix << "    Output: " << transition.output.summary() << "\n";
        result << prefix << "    Compatible: " << (transition.compatible ? "YES" : "NO");
        if (!transition.compatible) {
            result << " [INCOMPATIBLE INPUT]";
        }
        result << "\n";

        if (i < this->transitions.size() - 1) {
            result << "\n";
        }
    }

    // Final status
    if (!this->transitions.empty()) {
        result << "\n";
    }
    result << prefix << "Overall: " << (this->valid ? "VALID" : "INVALID") << "\n";

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