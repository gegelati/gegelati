#ifndef GRAPH_BASED_NODE_CONSTRAINT_H
#define GRAPH_BASED_NODE_CONSTRAINT_H

#include <vector>
#include "dimensions/constraint.h"
#include "dimensions/dataValueGenerator.h"

 namespace GraphBased {

    /**
     * \brief class representing the constraint of a GPNode.
     *
     * The class is essentially composed of a vector of constraints, one per value of the node.
     */
    class NodeConstraint {

        protected:
            /// @brief Vector of constraints for the node.
            std::vector<std::shared_ptr<const Dimensions::Constraint>> constraints;
        public:

            NodeConstraint() {};

            // Disable copying to avoid accidental copies (use references or pointers instead).
            NodeConstraint(const NodeConstraint&) = delete;
            NodeConstraint& operator=(const NodeConstraint&) = delete;


            /**
             * @brief Add the constraint of the next value of the node.
             *
             * \param constraint the constraint added.
             */
            void addConstraint(std::shared_ptr<const Dimensions::Constraint> constraint);

            /**
             * @brief Add the constraint of the next value of the node.
             *
             * \param constraint the constraint added.
             */
            void addConstraint(const Dimensions::Constraint& constraint);

            /**
             * \brief Get the Constraint at specified index
             * 
             * \param[in] idxValue specified index
             */
            const Dimensions::Constraint& getConstraintAt(size_t idxValue) const;

            /**
             * \brief get the number of Constraint
             */
            size_t size() const;

            /**
             * \brief Clone method sharing the constraints.
             */
            std::unique_ptr<NodeConstraint> cloneUniquePtr() const;
    };
}

#endif // GRAPH_BASED_NODE_CONSTRAINT_H