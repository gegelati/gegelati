#include "evolution/mutation.h"

Node::NodeValue Evolution::Mutation::sampleNodeValue(const std::vector<Node::NodeValueTemplate>& valueTemplates, RNG::RNG& rng)
{
    if(valueTemplates.size() == 0) {
        throw std::runtime_error("Evolution::Mutation::sampleNodeValue: NodeTemplates is empty :(.");
    }
    // Default select first template
    size_t idxTemplate = 0;

    // Sample a random template if more than one proposed
    if(valueTemplates.size() > 1) {
        idxTemplate = rng.getUnsignedInt64(0, valueTemplates.size() - 1);
    }

    const Node::NodeValueTemplate& valueTemplate = valueTemplates.at(idxTemplate);
    Node::NodeValue value;

    if(std::holds_alternative<Node::NodeRange>(valueTemplate)) {
        // Value should be selected in a specified range
        const Node::NodeRange& nodeRange = std::get<Node::NodeRange>(valueTemplate);

        if(std::holds_alternative<std::pair<size_t, size_t>>(nodeRange)) {
            // Range of unsigned int
            const std::pair<size_t, size_t>& uintPair = std::get<std::pair<size_t, size_t>>(nodeRange);
            value = rng.getUnsignedInt64(uintPair.first, uintPair.second - 1);

        } else if (std::holds_alternative<std::pair<double, double>>(nodeRange)) {
            // Range of double
            const std::pair<double, double>& doublePair = std::get<std::pair<double, double>>(nodeRange);
            value = rng.getDouble(doublePair.first, doublePair.second);
        } else {
            throw std::runtime_error("Evolution::Mutation::sampleNodeValue: NodeRange of unkown types.");
        }

    } else if (std::holds_alternative<std::vector<Node::NodeValue>>(valueTemplate)) {

        // List of node values.
        const std::vector<Node::NodeValue>& nodeValues = std::get<std::vector<Node::NodeValue>>(valueTemplate);

        // Sample a random value.
        value = nodeValues.at(rng.getUnsignedInt64(0, nodeValues.size() - 1));

    } else {
        throw std::runtime_error("Evolution::Mutation::sampleNodeValue: NodeValue of unkown types.");
    }

    return value;
}


std::unique_ptr<Node::GPNode> Evolution::Mutation::createRandomNode(const std::vector<Node::NodeValueTemplate>& genotypeTemplate, RNG::RNG& rng)
{
    
    std::vector<Node::NodeValue> values;
    for(const Node::NodeValueTemplate& valueTemplate: genotypeTemplate) {
        std::vector<std::vector<Node::NodeValueTemplate>> triche = {genotypeTemplate};
        values.push_back(this->sampleNodeValue(genotypeTemplate, rng));
    }
    return std::make_unique<Node::GPNode>(values);
}



void Evolution::Mutation::initRandomIndividual(Individual& indiv, const std::vector<Node::NodeValueTemplate>& genotypeTemplate, RNG::RNG& rng)
{
    size_t minNbNodes = 5;
    size_t maxNbNodes = 10;

    //for(auto& subStruct: structure) {
        size_t subNbNodes = rng.getUnsignedInt64(minNbNodes, maxNbNodes);
        for(size_t idxNode = 0; idxNode < subNbNodes; idxNode++) {
            std::unique_ptr<Node::GPNode> node(std::move(this->createRandomNode(genotypeTemplate, rng)));
            indiv.addGPNode(std::move(node));
        }
    //}
}
