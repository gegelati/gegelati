#include "representations/tpgRepresentation.h"

#include <sstream>
#include <limits>

std::unique_ptr<Evolution::Representation> Representations::TPGRepresentation::cloneUniquePtr() const
{
    auto clone = std::make_unique<Representations::TPGRepresentation>(
                                                        this->dimensionFlow.getInputDimensions(),
                                                        this->nbActions,
                                                        this->contextMemberRep,
                                                        this->contextMemberPop,
                                                        this->nbNodesMin,
                                                        this->nbNodesMax,
                                                        this->representationName,
                                                        this->representationColor
                                                    );
    /*if(this->tangledPopulation.has_value()) {
        clone->setTangledPopulation(this->tangledPopulation.value());
    }*/
    return clone;
}

/*

void Representations::TPGRepresentation::setGenotypeRequirements()
{

    if(!this->tangled || !this->tangledPopulation.has_value()) {
        throw std::runtime_error("Representations::TPGRepresentation::getGenotypeRequirements: cannot define if a tangled population is not set.");
    }

    Node::NodeRequirements bidNodesRequirements;

    // Value Requirements for members
    std::shared_ptr<Node::NodeValueConfiguration> configMember(
        std::make_shared<Node::NodeValueConfiguration>(this->contextMemberPop.getIndividualPtrs()));
    bidNodesRequirements->addValueRequirements(std::make_shared<Node::NodeValueRequirements>(configMember));
    
    // Value requirements for actions/Tangled connections

    // Action config
    std::vector<std::shared_ptr<const Node::NodeValueConfiguration>> configs;
    configs.push_back(
        std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), size_t(nbActions))));

    // Tangled config
    if(this->tangledPopulation->get().size() > 0) {
        configs.push_back(
            std::make_shared<Node::NodeValueConfiguration>(this->tangledPopulation->get().getIndividualPtrs()));
    }


    // Vector of both configs if tangled population is not empty
    bidNodesRequirements->addValueRequirements(std::make_shared<Node::NodeValueRequirements>(configs));

     

    this->genotypeRequirements.addNodeRequirements(bidNodesRequirements, this->nbNodesMin, this->nbNodesMax);
}*/


Data::DataValue Representations::TPGRepresentation::executeGenotype(
    const Evolution::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const
{
    // Get effective nodes
    std::vector<std::reference_wrapper<const Node::GPNode>> effectiveNodes = genotype.getEffectiveNodes().at(0);

    double maxBid = -std::numeric_limits<double>::infinity();
    size_t winnerIdx;

    for(size_t idx = 0; idx < effectiveNodes.size(); idx++) {
        const Node::GPNode& node = effectiveNodes.at(idx);
        const std::shared_ptr<const Evolution::Individual>& member = node.getValue(0).getScalar<std::shared_ptr<const Evolution::Individual>>();
        double bid = member->execute(inputSources).getScalar<double>();

        if(bid > maxBid) {
            maxBid = bid;
            winnerIdx = idx;
        }
    }

    if (typeid(size_t) == effectiveNodes.at(winnerIdx).get().getValue(1).getElementType()) {
        // Return action
        return effectiveNodes.at(winnerIdx).get().getValue(1).clone();
    } else {
        // Return action of tangled individual
        return effectiveNodes.at(winnerIdx).get().getValue(1).getScalar<std::shared_ptr<const Evolution::Individual>>()->execute(inputSources);
    }
}

std::unique_ptr<Node::GenotypeTemplate> Representations::TPGRepresentation::getGenotypeTemplate() const
{
    return nullptr;
}
