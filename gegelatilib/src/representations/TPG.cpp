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

Data::DataValue Representations::TPG::executeGenotype(
    const GraphBased::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const
{
    const GraphBased::NodeGroup& group = genotype.getNodeGroup(0);

    double maxBid = -std::numeric_limits<double>::infinity();
    double bid;
    const GraphBased::GPNode* winner = nullptr;
    const GraphBased::GPNode* firstEffective = nullptr;

    for(size_t idx = 0; idx < group.getSize(); idx++) {
        const GraphBased::GPNode& node = group.getNode(idx);
        if(node.getIsIntron()) {
            continue;
        }
        if(firstEffective == nullptr) {
            firstEffective = &node;
            winner = &node;
        }
        const std::shared_ptr<const Individual>& member = node.getValue(0).getScalar<std::shared_ptr<const Individual>>();
        bid = member->execute(inputSources).getScalar<double>();

        if(!std::isnan(bid) && bid > maxBid) {
            maxBid = bid;
            winner = &node;
        }
    }

    // Same as before: when no bid beats -inf, the first effective node wins (at() throws if none).
    if(firstEffective == nullptr) {
        throw std::out_of_range("Representations::TPG::executeGenotype: no effective node.");
    }

    if (typeid(size_t) == winner->getValue(1).getElementType()) {
        // Return action
        return winner->getValue(1).clone();
    } else {
        // Return action of tangled individual
        return winner->getValue(1).getScalar<std::shared_ptr<const Individual>>()->execute(inputSources);
    }
}
