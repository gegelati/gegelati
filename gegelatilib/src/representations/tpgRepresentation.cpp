#include "representations/tpgRepresentation.h"

#include <sstream>
#include <limits>

std::unique_ptr<Evolution::Representation> Representations::TPGRepresentation::cloneOnlyRepresentation() const
{
    auto clone = std::make_unique<Representations::TPGRepresentation>(
        this->dimensionFlow.getInputDimensions(), this->nbActions,
        this->nbNodesMin, this->nbNodesMax,
        this->representationName, this->representationColor
    );
    return clone;
}

void Representations::TPGRepresentation::setGenotypeConstraint()
{
   Node::NodeConstraint bidNodes;

    // Value Requirements for members
    bidNodes.addConstraint(Dimensions::NumericRange<double>::unbounded());

    // Action constraint
    bidNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->nbActions - 1));

    this->genotypeConstraint = std::make_unique<Node::GenotypeConstraint>(bidNodes, this->nbNodesMin, this->nbNodesMax);
}

void Representations::TPGRepresentation::setGenotypeGenerator()
{
   Node::NodeGenerator bidNodes;

    
    /* === Value requirements for members === */
    
    // Value Requirements for members
    bidNodes.addGenerator(Dimensions::ListUniformGenerator<std::shared_ptr<const Evolution::Individual>>(this->availableMembers));

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

        bidNodes.addGenerator(multi);
    } else {
        bidNodes.addGenerator(actionGenerator);
    }

    this->genotypeGenerator = std::make_unique<Node::GenotypeGenerator>(bidNodes, this->nbNodesMin, this->nbNodesMax);
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
        this->setGenotypeGenerator();
    }
}

std::unique_ptr<Node::GenotypeGenerator> Representations::TPGRepresentation::getGenotypeGenerator() const
{
    return std::move(this->genotypeGenerator->cloneUniquePtr());
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
