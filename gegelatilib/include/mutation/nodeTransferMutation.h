#ifndef NODE_TRANSFER_MUTATION
#define NODE_TRANSFER_MUTATION

#include "mutation/pointMutator.h"

namespace Mutation {
    /**
     * \brief Point mutation whose added nodes are copies of the nodes of other individuals.
     */
    class NodeTransferMutation : public PointMutator {

        protected:
            /// @brief Individuals potentially choosen for the node transfer.
            std::shared_ptr<const std::vector<std::shared_ptr<const Individual>>> potentialNodes;

        public:

            /**
             * \brief Constructor, see PointMutator.
             *
             * \param[in] pMutateNode Probability of mutating a node.
             * \param[in] pAddNode Probability of adding a node to the group if the generator allows it.
             * \param[in] pRemoveNode Probability of removing a node to the group if the generator allows it.
             * \param[in] name name of the mutator, only used to print it (e.g. in the summary of an evolution).
             */
            NodeTransferMutation(double pMutateNode, double pAddNode = 0.1, double pRemoveNode = 0.0, std::string name = "NodeTransferMutation")
            : PointMutator(pMutateNode, pAddNode, pRemoveNode, std::move(name)) {};

            /**
             * \brief Set the individuals whose nodes are transferred by the next mutations.
             *
             * The list is shared, not copied. Setting it is not thread safe, while mutating concurrently is.
             *
             * \param[in] potentialNodes individuals whose nodes can be transferred.
             */
            void setPotentialNodes(std::shared_ptr<const std::vector<std::shared_ptr<const Individual>>> potentialNodes);

            /**
             * \brief Override of the initial method to sample and dupplicate a random node in the current potential ones.
             */
            virtual void addNode(
                GraphBased::NodeGroup &mutatedGroup, size_t idxNodeGroup, const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const override;
    };
};

#endif // NODE_TRANSFER_MUTATION
