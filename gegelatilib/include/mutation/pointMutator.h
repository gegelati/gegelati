
#ifndef MUTATION_POINT_MUTATOR_H
#define MUTATION_POINT_MUTATOR_H

#include <vector>
#include <string>
#include <array>
#include <set>

#include "mutation/mutator.h"


 namespace Mutation {
    /**
     * \brief Class representing a Mutation.
     */
    class PointMutator : public Mutator
    {
    protected:

        /// @brief Probability of mutating a node.
        double pMutateNode;

        /// @brief Probability of adding a node to the group if the generator allows it.
        double pAddNode;

        /// @brief Probability of removing a node to the group if the generator allows it.
        double pRemoveNode;

    public:

        /// Default polymorphic destructor
        virtual ~PointMutator() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        PointMutator(const PointMutator&) = delete;
        PointMutator& operator=(const PointMutator&) = delete;
    
        /**
         * \brief Main Mutation constructor.
         * 
         * \param[in] pMutateNode Probability of mutating a node.
         * \param[in] pAddNode Probability of adding a node to the group if the generator allows it.
         * \param[in] pRemoveNode Probability of removing a node to the group if the generator allows it.
         */
        PointMutator(double pMutateNode, double pAddNode = 0.0, double pRemoveNode = 0.0): Mutator(), pMutateNode{pMutateNode}, pAddNode{pAddNode}, pRemoveNode{pRemoveNode} {};

        /**
         * \brief return a mutated copy of a GPNode
         *
         * \param[in] node the node to mutate.
         * \param[in] nodeGenerator node Generator of the node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateNode(GraphBased::GPNode& node, GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const override;

        /**
         * \brief return a mutated copy of a Genotype.
         *
         * \param[in,out] genotype the genotype mutated.
         * \param[in] genotypeGenerator genotype Generator of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<GraphBased::Genotype> mutateGenotype(const GraphBased::Genotype& genotype, std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator, RNG::RNG& rng) const override;

    };
}; // namespace Mutation

#endif // MUTATION_POINT_MUTATOR_H
