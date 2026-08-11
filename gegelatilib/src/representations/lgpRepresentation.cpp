#include "representations/lgpRepresentation.h"

std::unique_ptr<Evolution::Representation> Representations::LGPRepresentation::cloneUniquePtr() const
{
    return std::make_unique<Representations::LGPRepresentation>(*this);
}

bool Representations::LGPRepresentation::isValid(const Evolution::Individual& indiv)
{
    if(indiv.getSize() > this->nbNodesMax || indiv.getSize() < this->nbNodesMin) {
        return false;
    }

    std::vector<size_t> ranges = {this->nbRegisters, 4, 2, 8, 2, 8};
    for(const Node::GPNode& node: indiv.getEffectiveGenotype()) {
        if(node.getSize() != ranges.size()) {
            return false;
        }

        for(size_t idxNode = 0; idxNode < ranges.size(); idxNode++) {
            if(!std::holds_alternative<size_t>(node.getValue(idxNode))) {
                return false;
            }
            if(node.getValue(idxNode) >= Node::NodeType(ranges.at(idxNode))){
                return false;
            }
        }
    }

    return true;
}


std::vector<double> Representations::LGPRepresentation::executeIndividual(const Evolution::Individual& indiv)
{
    
}