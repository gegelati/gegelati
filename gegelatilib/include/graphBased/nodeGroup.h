#ifndef GRAPH_BASED_NODE_GROUP_H
#define GRAPH_BASED_NODE_GROUP_H

#include "graphBased/gpNode.h"

 namespace GraphBased {
    /**
     * \brief class storing a vector of nodes.
     */
    class NodeGroup {

        protected:
            /// @brief the vector of nodes contained in the group.
            std::vector<std::unique_ptr<GPNode>> nodes;

        public:
            // Disable copying to avoid accidental copies (use references or pointers instead).
            NodeGroup(const NodeGroup&) = delete;
            NodeGroup& operator=(const NodeGroup&) = delete;
            
            /// Default constructor
            NodeGroup() {};


            /**
             * \brief clone the current node group as a unique pointer.
             */
            virtual std::unique_ptr<NodeGroup> cloneUniquePtr() const;

            /**
             * \brief Add a Node to the NodeGroup.
             * 
             * \param[in] node the Node to add.
             * \param[in] index the index at which to add the Node.
             */
            virtual void addNode(std::unique_ptr<GPNode> node, size_t index);

            /**
             * \brief Add a Node to the NodeGroup.
             * 
             * \param[in] node the Node to add.
             */
            virtual void addNode(std::unique_ptr<GPNode> node);

            /**
             * \brief Remove a Node from the NodeGroup.
             * 
             * \param[in] index the index at which to remove the Node.
             */
            virtual void removeNode(size_t index);

            /**
             * \brief Get the Node at the given index.
             * 
             * \param[in] index the index of the Node to get.
             */
            virtual const GPNode& getNode(size_t index) const;

            /**
             * \brief Get the Node at the given index.
             * 
             * \param[in] index the index of the Node to get.
             */
            virtual GPNode& getNode(size_t index);

            /**
             * \brief Get the mutable Node at the given index.
             * 
             * \param[in] node the Node to set.
             * \param[in] index the index of the Node to get.
             */
            virtual void setNode(std::unique_ptr<GPNode> node, size_t index);

            /**
             * \brief Get the number of Node in the NodeGroup.
             */
            virtual size_t getSize() const;

            /**
             * \brief Get the nodes of the NodeGroup.
             */
            virtual std::vector<std::reference_wrapper<const GraphBased::GPNode>> getNodes() const;
    };

    /**
     * Override of equality operator
     */
    bool operator==(const GraphBased::NodeGroup& group1, const GraphBased::NodeGroup& group2);

    /**
     * Override of inequality operator
     */
    bool operator!=(const GraphBased::NodeGroup& group1, const GraphBased::NodeGroup& group2);
};

#endif // GRAPH_BASED_NODE_GROUP_H