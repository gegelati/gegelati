#include "representations/tpgRepresentation.h"
#include <limits>

std::unique_ptr<Evolution::Representation> Representations::TPGRepresentation::cloneUniquePtr() const
{
    std::unique_ptr<Evolution::Representation> clone = std::make_unique<Representations::TPGRepresentation>(
                                                        this->contextMemberRep,
                                                        this->contextMemberPop,
                                                        this->nbNodesMin,
                                                        this->nbNodesMax,
                                                        this->representationName,
                                                        this->representationColor
                                                    );
    if(this->tangledPopulation.has_value()) {
        clone->setTangledPopulation(this->tangledPopulation.value());
    }
    return std::move(clone);
}

void Representations::TPGRepresentation::setInputDimensions(const std::vector<Data::DataView>& inputSources)
{
    Evolution::Representation::setInputDimensions(inputSources);
}


std::unique_ptr<const Node::GenotypeTemplate> Representations::TPGRepresentation::getGenotypeTemplate() const
{
    if(this->nbInputSources == 0) {
        throw std::runtime_error("Representations::TPGRepresentation::getGenotypeTemplate: cannot define if an individual is valid without input dimensions set.");
    }
    if(!this->tangled || !this->tangledPopulation.has_value()) {
        throw std::runtime_error("Representations::TPGRepresentation::getGenotypeTemplate: cannot define if a tangled population is not set.");
    }

    size_t nbActions = 3;
    std::shared_ptr<Node::NodeTemplate> bidNodesTemplate = std::make_shared<Node::NodeTemplate>();


    // Value Template for members
    std::shared_ptr<Node::NodeValueConfiguration> configMember(
        std::make_shared<Node::NodeValueConfiguration>(this->contextMemberPop.getIndividualPtrs()));
    bidNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configMember));
    
    // Value template for actions/Tangled connections

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
    bidNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configs));

    return std::make_unique<Node::GenotypeTemplate>(
        bidNodesTemplate,
        std::make_pair(this->nbNodesMin, this->nbNodesMax)
    );
}

bool Representations::TPGRepresentation::isValid(const Evolution::Individual& indiv) const
{
    if(this->nbInputSources == 0) {
        throw std::runtime_error("Representations::TPGRepresentation::isValid: cannot define if an individual is valid without input dimensions set.");
    }
    if(!this->tangled || !this->tangledPopulation.has_value()) {
        throw std::runtime_error("Representations::TPGRepresentation::getGenotypeTemplate: cannot define if a tangled population is not set.");
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
        if(!std::holds_alternative<std::shared_ptr<const Evolution::Individual>>(node.getValue(0))){
            return false;
        }
        const std::shared_ptr<const Evolution::Individual>& member = std::get<std::shared_ptr<const Evolution::Individual>>(node.getValue(0));
        if(!this->contextMemberRep.isValid(*member)) {
            return false;
        }

        // Check Action/tangled individual
        bool isAction = false;
        if(std::holds_alternative<size_t>(node.getValue(1)) && std::get<size_t>(node.getValue(1)) < nbActions) {
            isAction = true;
        }

        bool isTangled = false;
        if(std::holds_alternative<std::shared_ptr<const Evolution::Individual>>(node.getValue(1))){
            const std::shared_ptr<const Evolution::Individual>& tangledIndiv = std::get<std::shared_ptr<const Evolution::Individual>>(node.getValue(1));
            if(indiv != *tangledIndiv && this->isValid(*tangledIndiv)) {
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
    const Evolution::Individual& indiv, const std::vector<Data::DataView>& inputSources) const
{
    // Get effective nodes
    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = indiv.getGenotype().getEffectiveNodes();

    double maxBid = -std::numeric_limits<double>::infinity();
    Node::NodeValue winner;

    for(const Node::GPNode& node: effectiveNodes.at(0)) {
        const std::shared_ptr<const Evolution::Individual>& member = std::get<std::shared_ptr<const Evolution::Individual>>(node.getValue(0));
        double bid = this->contextMemberRep.executeIndividual(*member, inputSources).at(0);

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
        return this->executeIndividual(*std::get<std::shared_ptr<const Evolution::Individual>>(winner), inputSources);
    }
}