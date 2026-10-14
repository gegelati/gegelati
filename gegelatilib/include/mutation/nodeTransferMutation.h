#ifndef NODE_TRANSFER_MUTATION
#define NODE_TRANSFER_MUTATION

#include "mutation/pointMutator.h"

namespace Mutation {
    /**
     * \brief Class representing a Mutation.
     */
    class NodeTransferMutation : public PointMutator {

        protected:
            /// @brief Individuals potentially choosen for the node transfer.
            std::vector<std::shared_ptr<const Individual>> potentialNodes;

        public:

            /// @brief Default constructor
            NodeTransferMutation(double pMutateNode, double pAddNode = 0.1, double pRemoveNode = 0.0) 
            : PointMutator(pMutateNode, pAddNode, pRemoveNode) {};


            /**
             * \brief the potential nodes for the next mutations
             */
            virtual void setPotentialNodes(const std::vector<std::shared_ptr<const Individual>>& potentialNodes);

            /**
             * \brief Override of the initial method to sample and dupplicate a random node in the current potential ones.
             */
            virtual void addNode(
                GraphBased::NodeGroup &mutatedGroup, size_t idxNodeGroup, const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const override;
    };
};

#endif // NODE_TRANSFER_MUTATION