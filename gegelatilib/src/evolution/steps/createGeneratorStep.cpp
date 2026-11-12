#include "evolution/steps/createGeneratorStep.h"

#include <stdexcept>

Evolution::CreateGeneratorStep::CreateGeneratorStep(std::string generator,
                                                    std::shared_ptr<const Representations::Representation> representation, std::string name)
    : CreateGeneratorStep(std::move(generator),
                          [representation](const EvolutionData&) { return std::make_shared<GraphBased::GenotypeGenerator>(representation->getGenotypeConstraint()); },
                          {}, std::move(name))
{
    if(representation == nullptr) {
        throw std::runtime_error("Evolution::CreateGeneratorStep: the representation cannot be null");
    }
}

Evolution::CreateGeneratorStep::CreateGeneratorStep(std::string generator,
                                                    std::function<std::shared_ptr<GraphBased::GenotypeGenerator>(const EvolutionData&)> build,
                                                    std::set<std::string> required, std::string name)
    : Module(std::move(name)), generator{std::move(generator)}, build{std::move(build)}
{
    this->provide(this->generator);
    for(const std::string& requiredName: required) {
        this->require(requiredName);
    }
    if(this->build == nullptr) {
        throw std::runtime_error("Evolution::CreateGeneratorStep: the function building the generator cannot be empty");
    }
}

std::string Evolution::CreateGeneratorStep::details() const
{
    return this->generator;
}

void Evolution::CreateGeneratorStep::apply(EvolutionData& state)
{
    std::shared_ptr<GraphBased::GenotypeGenerator> created = this->build(state);
    if(created == nullptr) {
        throw std::runtime_error("Evolution::CreateGeneratorStep: the function building the generator returned null");
    }
    // The generator is created by the initialization, then replaced when the step runs again.
    if(state.generation == EvolutionData::initGeneration) {
        state.add(this->generator, std::move(created));
    } else {
        state.set(this->generator, std::move(created));
    }
}
