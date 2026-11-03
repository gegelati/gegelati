#ifndef TPG_GENERATOR_H
#define TPG_GENERATOR_H

#include <memory>
#include <vector>

#include "dimensions/dataValueGenerator.h"
#include "graphBased/genotypeGenerator.h"
#include "individual.h"
#include "representations/TPG.h"

/**
 * Generator of the genotypes of a TPG for the tests: members bid in the nodes, and the destination is an action or,
 * if there are tangled individuals, sometimes one of them. The lists must outlive the generator.
 */
inline std::unique_ptr<GraphBased::GenotypeGenerator> tpgGenerator(const Representations::TPG& tpg, const std::vector<std::shared_ptr<const Individual>>& members,
                                                                   const std::vector<std::shared_ptr<const Individual>>& tangled)
{
    GraphBased::NodeGenerator bidNodes;
    bidNodes.addGenerator(Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>>(members));
    Dimensions::NumericUniformGenerator<size_t> actionGenerator(0, tpg.getNbActions() - 1);
    if(tangled.empty()) {
        bidNodes.addGenerator(actionGenerator);
    } else {
        Dimensions::MultiGenerator multi;
        multi.addGenerator(actionGenerator, 0.999);
        multi.addGenerator(Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>>(tangled), 0.001);
        bidNodes.addGenerator(multi);
    }
    return std::make_unique<GraphBased::GenotypeGenerator>(bidNodes, tpg.getGenotypeConstraint().getRangeAt(0));
}

#endif // TPG_GENERATOR_H
