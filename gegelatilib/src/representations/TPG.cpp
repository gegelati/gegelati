#include "representations/TPG.h"

#include <sstream>
#include <limits>

std::unique_ptr<Representations::Representation> Representations::TPG::cloneOnlyRepresentation() const
{
    auto clone = std::make_unique<Representations::TPG>(
        this->dimensionFlow.getInputDimensions(), this->nbActions,
        this->nbMembersMin, this->nbMembersMax,
        this->representationName, this->representationColor
    );
    return clone;
}

const size_t& Representations::TPG::getNbActions() const
{
    return this->nbActions;
}

void Representations::TPG::setGenotypeConstraint()
{
   GraphBased::NodeConstraint bidNodes;

    // Value Requirements for members
    bidNodes.addConstraint(Dimensions::NumericRange<double>::unbounded());

    // Action constraint
    bidNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->nbActions - 1));

    this->genotypeConstraint = std::make_unique<GraphBased::GenotypeConstraint>(bidNodes, this->nbMembersMin, this->nbMembersMax);
}

void Representations::TPG::setGenotypeGenerator()
{
   GraphBased::NodeGenerator bidNodes;
    /* === Value requirements for members === */
    
    // Value Requirements for members
    bidNodes.addGenerator(Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>>(this->availableMembers));

    /* === Value requirements for actions/Tangled connections === */

    // Action generator
    Dimensions::NumericUniformGenerator<size_t> actionGenerator(0, this->nbActions - 1);
    
    // Add the generator for tangled individuals only if it is not empty
    if(this->availableForTangled.size() > 0) {
        // Tangled generator
        Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>> tangledGenerator(this->availableForTangled);

        // Multi generator
        Dimensions::MultiGenerator multi;
        multi.addGenerator(actionGenerator, 0.999);
        multi.addGenerator(tangledGenerator, 0.001);

        bidNodes.addGenerator(multi);
    } else {
        bidNodes.addGenerator(actionGenerator);
    }

    this->genotypeGenerator = std::make_unique<GraphBased::GenotypeGenerator>(bidNodes, std::make_pair(this->nbMembersMin, this->nbMembersMax));
}

void Representations::TPG::setAvailableMembers(const std::vector<std::shared_ptr<const Individual>>& members)
{
    this->availableMembers.clear();
    this->availableMembers.insert(this->availableMembers.begin(), members.begin(), members.end());
}


void Representations::TPG::setAvailableTangledIndiv(const std::vector<std::shared_ptr<const Individual>>& tangledIndiv)
{
    bool isEmpty = this->availableForTangled.empty();
    this->availableForTangled.clear();
    this->availableForTangled.insert(this->availableForTangled.begin(), tangledIndiv.begin(), tangledIndiv.end());

    // If the emptyness value changed, reset the template
    if((isEmpty != this->availableForTangled.empty())) {
        this->setGenotypeGenerator();
    }
}

Data::DataValue Representations::TPG::executeGenotype(
    const GraphBased::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const
{
    const GraphBased::NodeGroup& group = genotype.getNodeGroup(0);

    double maxBid = -std::numeric_limits<double>::infinity();
    const GraphBased::GPNode* winner = nullptr;
    const GraphBased::GPNode* firstEffective = nullptr;
    size_t nbBids = 0;

    for(size_t idx = 0; idx < group.getSize(); idx++) {
        const GraphBased::GPNode& node = group.getNode(idx);
        if(node.getIsIntron()) {
            continue;
        }
        if(firstEffective == nullptr) {
            firstEffective = &node;
        }
        const std::shared_ptr<const Individual>& member = node.getValue(0).getScalar<std::shared_ptr<const Individual>>();
        double bid = member->execute(inputSources).getScalar<double>();
        nbBids++;

        if(bid > maxBid) {
            maxBid = bid;
            winner = &node;
        }
    }

    // Same as before: when no bid beats -inf, the first effective node wins (at() throws if none).
    if(winner == nullptr) {
        if(firstEffective == nullptr) {
            throw std::out_of_range("Representations::TPG::executeGenotype: no effective node.");
        }
        winner = firstEffective;
    }

    if (typeid(size_t) == winner->getValue(1).getElementType()) {
        // Return action
        return winner->getValue(1).clone();
    } else {
        // Return action of tangled individual
        return winner->getValue(1).getScalar<std::shared_ptr<const Individual>>()->execute(inputSources);
    }
}
