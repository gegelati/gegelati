#include "evolution/steps/createGeneratorStep.h"

#include <stdexcept>

Evolution::CreateGeneratorStep::CreateGeneratorStep(Key<GraphBased::GenotypeGenerator> generator,
                                                    std::shared_ptr<const Representations::Representation> representation, std::string name)
    : CreateGeneratorStep(std::move(generator),
                          [representation](const EvolutionData&) { return std::make_shared<GraphBased::GenotypeGenerator>(representation->getGenotypeConstraint()); },
                          {}, std::move(name))
{
    if(representation == nullptr) {
        throw std::runtime_error("Evolution::CreateGeneratorStep: the representation cannot be null");
    }
}

Evolution::CreateGeneratorStep::CreateGeneratorStep(Key<GraphBased::GenotypeGenerator> generator,
                                                    std::function<std::shared_ptr<GraphBased::GenotypeGenerator>(const EvolutionData&)> build,
                                                    std::set<std::string> required, std::string name)
    : Module(std::move(name)), generator{std::move(generator)}, build{std::move(build)}
{
    this->provide(this->generator);
    for(const std::string& name: required) {
        this->require(name);
    }
    if(this->build == nullptr) {
        throw std::runtime_error("Evolution::CreateGeneratorStep: the function building the generator cannot be empty");
    }
}

std::string Evolution::CreateGeneratorStep::details() const
{
    return this->generator.name();
}

void Evolution::CreateGeneratorStep::apply(EvolutionData& state)
{
    std::shared_ptr<GraphBased::GenotypeGenerator> created = this->build(state);
    if(created == nullptr) {
        throw std::runtime_error("Evolution::CreateGeneratorStep: the function building the generator returned null");
    }
    state.set(this->generator, std::move(created));
}
