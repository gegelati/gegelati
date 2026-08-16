#include "representations/tPGRepresentation.h"
#include <limits>

std::unique_ptr<Evolution::Representation> Representations::TPGRepresentation::cloneUniquePtr() const
{
    return std::make_unique<Representations::TPGRepresentation>(
        this->contextMemberRep,
        this->contextMemberPop,
        this->tangledPopulation,
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
    std::vector<Node::NodeValue> nodeValueMemberIndividuals(memberIndividuals.begin(), memberIndividuals.end());
    std::shared_ptr<Node::NodeValueConfiguration> configMember(
        std::make_shared<Node::NodeValueConfiguration>(nodeValueMemberIndividuals));
    bidNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configMember));
    
    // Value template for actions/Tangled connections

    // Action config
    std::shared_ptr<Node::NodeValueConfiguration> configActions(
        std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), size_t(nbActions))));

    // Tangled config
    std::vector<std::reference_wrapper<const Evolution::Individual>> tangledIndividuals(this->tangledPopulation.getIndividuals());
    std::vector<Node::NodeValue> nodeValueTangledIndividuals(tangledIndividuals.begin(), tangledIndividuals.end());
    std::shared_ptr<Node::NodeValueConfiguration> configTangled(
        std::make_shared<Node::NodeValueConfiguration>(nodeValueTangledIndividuals));

    // Vector of both configs
    std::vector<std::shared_ptr<const Node::NodeValueConfiguration>> configs{configActions, configTangled};
    bidNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configs));

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

        // Check member individual
        if(!std::holds_alternative<std::reference_wrapper<const Evolution::Individual>>(node.getValue(0))){
            return false;
        }
        const Evolution::Individual& member = std::get<std::reference_wrapper<const Evolution::Individual>>(node.getValue(0));
        if(!this->contextMemberRep.isValid(member)) {
            return false;
        }

        // Check Action/tangled individual
        bool isAction = false;
        if(std::holds_alternative<size_t>(node.getValue(1)) && std::get<size_t>(node.getValue(1)) < nbActions) {
            isAction = true;
        }

        bool isTangled = false;
        if(std::holds_alternative<std::reference_wrapper<const Evolution::Individual>>(node.getValue(1))){
            const Evolution::Individual& tangledIndiv = std::get<std::reference_wrapper<const Evolution::Individual>>(node.getValue(1));
            if(this->isValid(tangledIndiv) && indiv != tangledIndiv) {
                isTangled = true;
            }
        }
        if(!isAction && !isTangled) {
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
    Node::NodeValue winner;

    for(const Node::GPNode& node: effectiveNodes.at(0)) {
        const Evolution::Individual& member = std::get<std::reference_wrapper<const Evolution::Individual>>(node.getValue(0));
        double bid = this->contextMemberRep.executeIndividual(member, inputSources).at(0);

        if(bid > maxBid) {
            maxBid = bid;
            winner = node.getValue(1);
        }
    }

    if (std::holds_alternative<size_t>(winner)) {
        // Return action
        return {double(std::get<size_t>(winner))};
    } else {
        // Return action of tangled individual
        return this->executeIndividual(std::get<std::reference_wrapper<const Evolution::Individual>>(winner), inputSources);
    }
}