#include "representations/tPGRepresentation.h"
#include <limits>

std::unique_ptr<Evolution::Representation> Representations::TPGRepresentation::cloneUniquePtr() const
{
    return std::make_unique<Representations::TPGRepresentation>(
        this->contextMemberRep,
        this->contextMemberPop,
        this->nbNodesMin,
        this->nbNodesMax,
        this->representationName,
        this->representationColor
    );
}

void Representations::TPGRepresentation::setInputDimensions(const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources)
{
    Evolution::Representation::setInputDimensions(inputSources);
}


std::unique_ptr<const Node::GenotypeTemplate> Representations::TPGRepresentation::getGenotypeTemplate() const
{
    if(this->nbInputSources == 0 || this->maxInputSourceIdx == 0) {
        throw std::runtime_error("Representations::TPGRepresentation::getGenotypeTemplate: cannot define if an individual is valid without input dimensions set.");
    }

    size_t nbActions = 3;
    std::shared_ptr<Node::NodeTemplate> bidNodesTemplate = std::make_shared<Node::NodeTemplate>();


    // Value Template for members
    std::vector<std::reference_wrapper<const Evolution::Individual>> memberIndividuals(this->contextMemberPop.getIndividuals());
    std::vector<Node::NodeValue> nodeValueIndividuals(memberIndividuals.begin(), memberIndividuals.end());
    std::shared_ptr<Node::NodeValueConfiguration> configMember(
        std::make_shared<Node::NodeValueConfiguration>(nodeValueIndividuals));
    bidNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configMember));
    
    // Value template for actions
    std::shared_ptr<Node::NodeValueConfiguration> configActions(
        std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), size_t(nbActions))));
    bidNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configActions));

    return std::make_unique<Node::GenotypeTemplate>(
        bidNodesTemplate,
        std::make_pair(this->nbNodesMin, this->nbNodesMax)
    );
}

bool Representations::TPGRepresentation::isValid(const Evolution::Individual& indiv) const
{
    if(this->nbInputSources == 0 || this->maxInputSourceIdx == 0) {
        throw std::runtime_error("Representations::TPGRepresentation::isValid: cannot define if an individual is valid without input dimensions set.");
    }

    // Return false if genotype length is out of bounds.
    if(indiv.getSize() > this->nbNodesMax || indiv.getSize() < this->nbNodesMin) {
        return false;
    }
    size_t nbActions = 3;

    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = indiv.getGenotype().getEffectiveNodes();

    // Verify each (effective) node corresponds to the required specifications.
    for(const Node::GPNode& node: effectiveNodes.at(0)) {
        if(node.getSize() != 2) {
            return false;
        }

        if(!std::holds_alternative<std::reference_wrapper<const Evolution::Individual>>(node.getValue(0))){
            return false;
        }
        const Evolution::Individual& member = std::get<std::reference_wrapper<const Evolution::Individual>>(node.getValue(0));
        if(!this->contextMemberRep.isValid(member) || !this->contextMemberPop.containsIndividual(member)) {
            return false;
        }

        if(!std::holds_alternative<size_t>(node.getValue(1)) || std::get<size_t>(node.getValue(1)) >= nbActions) {
            return false;
        }
    }
    return true;
}


std::vector<double> Representations::TPGRepresentation::executeIndividual(
    const Evolution::Individual& indiv, const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) const
{
    // Get effective nodes
    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = indiv.getGenotype().getEffectiveNodes();

    double maxBid = -std::numeric_limits<double>::infinity();
    size_t action = 0;

    for(const Node::GPNode& node: effectiveNodes.at(0)) {
        const Evolution::Individual& member = std::get<std::reference_wrapper<const Evolution::Individual>>(node.getValue(0));
        double bid = this->contextMemberRep.executeIndividual(member, inputSources).at(0);

        if(bid > maxBid) {
            maxBid = bid;
            action = std::get<size_t>(node.getValue(1));
        }
    }

    // Return action
    return {double(action)};
}