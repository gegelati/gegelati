#include "mutation/nodeTransferMutation.h"

void Mutation::NodeTransferMutation::setPotentialNodes(std::shared_ptr<const std::vector<std::shared_ptr<const Individual>>> potentialNodes)
{
    this->potentialNodes = std::move(potentialNodes);
}

void Mutation::NodeTransferMutation::addNode(
    GraphBased::NodeGroup &mutatedGroup, size_t idxNodeGroup, const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    if(this->potentialNodes == nullptr || this->potentialNodes->empty()) {
        throw std::runtime_error("Mutation::NodeTransferMutation::addNode: Cannot add a node because potential nodes list is empty");
    }

    std::vector<std::reference_wrapper<const GraphBased::GPNode>> allNodes;

    for(const std::shared_ptr<const Individual>& indiv: *this->potentialNodes) {
        const GraphBased::NodeGroup& group = indiv->getGenotype().getNodeGroup(idxNodeGroup);
        for (size_t idxNode = 0; idxNode < group.getSize(); idxNode++) {
            allNodes.push_back(group.getNode(idxNode));
        }
    }

    const GraphBased::GPNode& selectedNode = allNodes.at(rng.getUnsignedInt64(0, allNodes.size() - 1));
    mutatedGroup.addNode(selectedNode.cloneUniquePtr());
}
