#include "evolution/mutation.h"

#include "evolution/individual.h"


std::unique_ptr<GraphBased::GPNode> Evolution::Mutation::createRandomNode(GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    if(nodeGenerator.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::createRandomNode: NodeGenerator is empty.");
    }

    std::vector<Data::DataValue> values;
    for(size_t idxValue = 0; idxValue < nodeGenerator.size(); idxValue++) {
        values.push_back(nodeGenerator.getGeneratorAt(idxValue).cloneUniquePtr()->sample(rng));
    }
    return std::make_unique<GraphBased::GPNode>(values);
}


std::unique_ptr<Evolution::Genotype> Evolution::Mutation::initRandomGenotype(std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator, RNG::RNG& rng) const
{
    if(genotypeGenerator->size() == 0) {
        throw std::runtime_error("Evolution::Mutation::initRandomGenotype: genotypeGenerator is empty.");
    }
    std::unique_ptr<Genotype> genotype = std::make_unique<Genotype>();

    for(size_t idxGenerator = 0; idxGenerator < genotypeGenerator->size(); idxGenerator++) {
        std::unique_ptr<GraphBased::NodeGroup> nodeGroup = std::make_unique<GraphBased::NodeGroup>();

        const std::pair<size_t, size_t>& range = genotypeGenerator->getRangeAt(idxGenerator);
        size_t nbNodesOfGenerator = rng.getUnsignedInt64(range.first, range.second);

        for(size_t idxNode = 0; idxNode < nbNodesOfGenerator; idxNode++) {
            nodeGroup->addNode(this->createRandomNode(genotypeGenerator->getNodeGeneratorAt(idxGenerator), rng));
        }
        genotype->addNodeGroup(std::move(nodeGroup));
    }
    return std::move(genotype);
}

std::shared_ptr<Evolution::Individual> Evolution::Mutation::initIndividual(const Representation& representation, RNG::RNG& rng) const
{
    return std::make_shared<Evolution::Individual>(
        representation, 
        this->initRandomGenotype(representation.getGenotypeGenerator(), rng)
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

void Evolution::Mutation::mutateNode(GraphBased::GPNode& node, GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    if(nodeGenerator.size() != node.getSize()) {
        throw std::runtime_error("Evolution::Mutation::mutateNode: NodeGenerator size does not correspond to the genotypeidual.");
    }
    // Sample random index
    size_t idxValueMutated = rng.getUnsignedInt64(0, node.getSize() - 1);

    // Sample random value for index
    Data::DataValue newValue = nodeGenerator.getGeneratorAt(idxValueMutated).sample(rng);

    // Copy node with assigned value
    node.setValue(idxValueMutated, newValue);
}


std::unique_ptr<Evolution::Genotype> Evolution::Mutation::mutateGenotype(const Genotype& genotype, std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator, RNG::RNG& rng) const
{
    if(genotypeGenerator->size() == 0) {
        throw std::runtime_error("Evolution::Mutation::mutateGenotype: genotypeGenerator is empty.");
    }
    std::unique_ptr<Evolution::Genotype> mutatedGenotype = genotype.cloneUniquePtr();

    double pMutateNode = 0.5;

    GraphBased::NodeGenerator& nodeGenerator = genotypeGenerator->getNodeGeneratorAt(0);
    for(size_t idxNodeGroup = 0; idxNodeGroup < genotype.getSize(); idxNodeGroup++) {
        GraphBased::NodeGroup& mutatedGroup = mutatedGenotype->getNodeGroup(idxNodeGroup);

        for(size_t idxNode = 0; idxNode < mutatedGroup.getSize(); idxNode++) {
            if(rng.getDouble(0, 1) < pMutateNode) {
                this->mutateNode(mutatedGroup.getNode(idxNode), nodeGenerator, rng);
            }
        }
    }
    return std::move(mutatedGenotype);
}

void Evolution::Mutation::mutateIndividuals(std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals, RNG::RNG& rng) const
{
    for(const std::shared_ptr<Evolution::Individual>& indiv: individuals) {
        std::unique_ptr<Evolution::Genotype> mutatedGenotype 
            = std::move(this->mutateGenotype(indiv->getGenotype(), indiv->getRepresentation().getGenotypeGenerator(), rng));
        indiv->setGenotype(std::move(mutatedGenotype));
    }
}