#include "representations/tpgRepresentation.h"

#include <sstream>
#include <limits>

std::unique_ptr<Evolution::Representation> Representations::TPGRepresentation::cloneUniquePtr() const
{
    auto clone = std::make_unique<Representations::TPGRepresentation>(
        this->dimensionFlow.getInputDimensions(), this->nbActions,
        this->nbNodesMin, this->nbNodesMax,
        this->representationName, this->representationColor
    );
    return clone;
}

void Representations::TPGRepresentation::setGenotypeTemplate()
{
   Node::NodeTemplate bidNodes;

    
    /* === Value requirements for members === */
    
    // Value Requirements for members
    bidNodes.addTemplate(
        Dimensions::NumericRange<double>::unbounded(),
        Dimensions::ListUniformGenerator<std::shared_ptr<const Evolution::Individual>>(this->availableMembers)
    );

    /* === Value requirements for actions/Tangled connections === */

    // Action generator
    Dimensions::NumericUniformGenerator<size_t> actionGenerator(0, this->nbActions - 1);
    
    // Add the generator for tangled individuals only if it is not empty
    if(this->availableForTangled.size() > 0) {
        // Tangled generator
        Dimensions::ListUniformGenerator<std::shared_ptr<const Evolution::Individual>> tangledGenerator(this->availableForTangled);

        // Multi generator
        Dimensions::MultiGenerator multi;
        multi.addGenerator(actionGenerator, 0.5);
        multi.addGenerator(tangledGenerator, 0.5);

        bidNodes.addTemplate(Dimensions::NumericRange<size_t>::between(0, this->nbActions - 1), multi);
    } else {
        bidNodes.addTemplate(Dimensions::NumericRange<size_t>::between(0, this->nbActions - 1), actionGenerator);
    }

    this->genotypeTemplate = std::make_unique<Node::GenotypeTemplate>();
    this->genotypeTemplate->addNodeTemplate(bidNodes, this->nbNodesMin, this->nbNodesMax);
}

void Representations::TPGRepresentation::setAvailableMembers(const std::vector<std::shared_ptr<const Evolution::Individual>>& members)
{
    this->availableMembers.clear();
    this->availableMembers.insert(this->availableMembers.begin(), members.begin(), members.end());
}


void Representations::TPGRepresentation::setAvailableTangledIndiv(const std::vector<std::shared_ptr<const Evolution::Individual>>& tangledIndiv)
{
    bool isEmpty = this->availableForTangled.empty();
    this->availableForTangled.clear();
    this->availableForTangled.insert(this->availableForTangled.begin(), tangledIndiv.begin(), tangledIndiv.end());

    // If the emptyness value changed, reset the template
    if((isEmpty != this->availableForTangled.empty())) {
        this->setGenotypeTemplate();
    }
}

std::unique_ptr<Node::GenotypeTemplate> Representations::TPGRepresentation::getGenotypeTemplate() const
{
    if(this->availableMembers.size() == 0) {
        throw std::runtime_error("Representations::TPGRepresentation::getGenotypeTemplate: list of member individuals cannot be empty.");
    }
    return std::move(this->genotypeTemplate->cloneUniquePtr());
}


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
