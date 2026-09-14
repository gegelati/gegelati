#include "evolution/mutation.h"

#include "evolution/individual.h"


std::unique_ptr<Node::GPNode> Evolution::Mutation::createRandomNode(Node::NodeTemplate& nodeTemplate, RNG::RNG& rng) const
{
    if(nodeTemplate.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::createRandomNode: NodeTemplate is empty.");
    }

    std::vector<Data::DataValue> values;
    for(size_t idxValue = 0; idxValue < nodeTemplate.size(); idxValue++) {
        values.push_back(nodeTemplate.getGeneratorAt(idxValue).cloneUniquePtr()->sample(rng));
    }
    return std::make_unique<Node::GPNode>(values);
}


std::unique_ptr<Evolution::Genotype> Evolution::Mutation::initRandomGenotype(std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng) const
{
    if(genotypeTemplate->size() == 0) {
        throw std::runtime_error("Evolution::Mutation::initRandomGenotype: genotypeTemplate is empty.");
    }
    std::unique_ptr<Genotype> genotype = std::make_unique<Genotype>();

    for(size_t idxTemplate = 0; idxTemplate < genotypeTemplate->size(); idxTemplate++) {
        std::unique_ptr<Node::NodeGroup> nodeGroup = std::make_unique<Node::NodeGroup>();

        const std::pair<size_t, size_t>& range = genotypeTemplate->getRangeAt(idxTemplate);
        size_t nbNodesOfTemplate = rng.getUnsignedInt64(range.first, range.second);

        for(size_t idxNode = 0; idxNode < nbNodesOfTemplate; idxNode++) {
            nodeGroup->addNode(this->createRandomNode(genotypeTemplate->getNodeTemplateAt(idxTemplate), rng));
        }
        genotype->addNodeGroup(std::move(nodeGroup));
    }
    return std::move(genotype);
}

std::shared_ptr<Evolution::Individual> Evolution::Mutation::initIndividual(const Representation& representation, RNG::RNG& rng) const
{
    return std::make_shared<Evolution::Individual>(
        representation, 
        this->initRandomGenotype(representation.getGenotypeTemplate(), rng)
    );
}

std::set<std::shared_ptr<Evolution::Individual>, SharedLess<Evolution::Individual>> 
    Evolution::Mutation::initIndividuals(const Evolution::Representation& representation, size_t nbIndividuals, RNG::RNG& rng) const
{
    std::set<std::shared_ptr<Evolution::Individual>, SharedLess<Evolution::Individual>> individuals;
    for(size_t idx = 0; idx < nbIndividuals; idx++) {
        individuals.insert(this->initIndividual(representation, rng));
    }
    return individuals;
}

void Evolution::Mutation::mutateNode(Node::GPNode& node, Node::NodeTemplate& nodeTemplate, RNG::RNG& rng) const
{
    if(nodeTemplate.size() != node.getSize()) {
        throw std::runtime_error("Evolution::Mutation::mutateNode: NodeTemplate size does not correspond to the genotypeidual.");
    }
    // Sample random index
    size_t idxValueMutated = rng.getUnsignedInt64(0, node.getSize() - 1);

    // Sample random value for index
    Data::DataValue newValue = nodeTemplate.getGeneratorAt(idxValueMutated).sample(rng);

    // Copy node with assigned value
    node.setValue(idxValueMutated, newValue);
}


std::unique_ptr<Evolution::Genotype> Evolution::Mutation::mutateGenotype(const Genotype& genotype, std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng) const
{
    if(genotypeTemplate->size() == 0) {
        throw std::runtime_error("Evolution::Mutation::mutateGenotype: genotypeTemplate is empty.");
    }
    std::unique_ptr<Evolution::Genotype> mutatedGenotype = genotype.cloneUniquePtr();

    double pMutateNode = 0.5;

    Node::NodeTemplate& nodeTemplate = genotypeTemplate->getNodeTemplateAt(0);
    for(size_t idxNodeGroup = 0; idxNodeGroup < genotype.getSize(); idxNodeGroup++) {
        Node::NodeGroup& mutatedGroup = mutatedGenotype->getNodeGroup(idxNodeGroup);

        for(size_t idxNode = 0; idxNode < mutatedGroup.getSize(); idxNode++) {
            if(rng.getDouble(0, 1) < pMutateNode) {
                this->mutateNode(mutatedGroup.getNode(idxNode), nodeTemplate, rng);
            }
        }
    }
    return std::move(mutatedGenotype);
}

void Evolution::Mutation::mutateIndividuals(std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals, RNG::RNG& rng) const
{
    for(const std::shared_ptr<Evolution::Individual>& indiv: individuals) {
        std::unique_ptr<Evolution::Genotype> mutatedGenotype 
            = std::move(this->mutateGenotype(indiv->getGenotype(), indiv->getRepresentation().getGenotypeTemplate(), rng));
        indiv->setGenotype(std::move(mutatedGenotype));
    }
}