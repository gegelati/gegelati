#include "evolution/mutation.h"

void Evolution::Mutation::initRandomIndividual(Individual& indiv, RNG::RNG& rng)
{
    //structure = this->representation->getNodesStructure;
    std::vector<std::vector<size_t>> structure = 
    {
        {5, 10,   8, 4, 2, 8, 2, 8}
    };

    for(auto& subStruct: structure) {
        size_t subNbNodes = rng.getUnsignedInt64(subStruct.at(0), subStruct.at(1));
        for(size_t idxNode = 0; idxNode < subNbNodes; idxNode++) {
            std::unique_ptr<Node::GPNode> node(std::move(this->createRandomNode(rng)));
            indiv.addGPNode(std::move(node));
        }
    }
}


std::unique_ptr<Node::GPNode> Evolution::Mutation::createRandomNode(RNG::RNG& rng)
{
    std::vector<size_t> maxRanges =  {8, 4, 2, 8, 2, 8};
    std::vector<size_t> values;
    for(size_t maxRange: maxRanges) {
        values.push_back(rng.getUnsignedInt64(0, maxRange-1));
    }
    return std::make_unique<Node::GPNode>(values);
}