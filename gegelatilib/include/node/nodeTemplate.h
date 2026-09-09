#ifndef NODE_TEMPLATE_H
#define NODE_TEMPLATE_H

#include <vector>
#include "dimensions/constraint.h"
#include "dimensions/dataValueGenerator.h"

namespace Node {

    /**
     * \brief class representation the template of a GPNode.
     * 
     * The class is essentially composed on a vector of constraints and a vector of generators.
     */
    class NodeTemplate {

        protected:
            /// @brief Vector of constraints for the node.
            std::vector<std::shared_ptr<const Dimensions::Constraint>> constraints;
            /// @brief Vector of generators for the node.
            std::vector<std::unique_ptr<Dimensions::DataValueGenerator>> generators;
        public:

            NodeTemplate() {};

            // Disable copying to avoid accidental copies (use references or pointers instead).
            NodeTemplate(const NodeTemplate&) = delete;
            NodeTemplate& operator=(const NodeTemplate&) = delete;


            /**
             * @brief Add a new constraint and generator to the template.
             * 
             * \param constraint the constraint to add to the template.
             * \param generator the generator to add to the template.
             */
            void addTemplate(const Dimensions::Constraint& constraint, const Dimensions::DataValueGenerator& generator);

            /**
             * \brief Get the Constraint at specified index
             * 
             * \param[in] idxValue specified index
             */
            const Dimensions::Constraint& getConstraintAt(size_t idxValue) const;

            /**
             * \brief Get the DataValueGenerator at specified index
             * 
             * \param[in] idxValue specified index
             */
            Dimensions::DataValueGenerator& getGeneratorAt(size_t idxValue);

            /**
             * \brief get the number of Template
             */
            size_t size() const;

            /**
             * \brief Clone method returning the same shared pointer of constraints and dupplicated unique pointer of generators
             */
            std::unique_ptr<NodeTemplate> cloneUniquePtr() const;
    };
}

#endif // NODE_TEMPLATE_H