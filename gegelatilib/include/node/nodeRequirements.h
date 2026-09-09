#ifndef NODE_REQUIREMENTS_H
#define NODE_REQUIREMENTS_H

#include "node/nodeValueRequirement.h"

namespace Node {

    /**
     * \brief class representation the requirements of a GPNode.
     * 
     * The class is essentially composed on a vector of nodeValueRequirement, each one indicating the requirements of a value in the node.
     */
    class NodeRequirements {

        protected:
            /// @brief Vector of NodeValueRequirement for the node.
            std::vector<NodeValueRequirement> nodeValueRequirements;
        public:
            /**
             * @brief Default requirements constructor
             */
            NodeRequirements() {};

            /**
             * @brief Constructor with a vector of NodeValueRequirement
             * 
             * \param[in] nodeValueRequirement single NodeValueRequirement to initialize the NodeRequirements
             */
            NodeRequirements(const NodeValueRequirement nodeValueRequirement) : nodeValueRequirements{nodeValueRequirement} {};

            /**
             * @brief Constructor with a vector of NodeValueRequirement
             * 
             * \param[in] nodeValueRequirements vector of NodeValueRequirement to initialize the NodeRequirements
             */
            NodeRequirements(const std::vector<NodeValueRequirement>& nodeValueRequirements) : nodeValueRequirements{nodeValueRequirements} {};

            /**
             * @brief Add a new NodeValueRequirement to the requirements.
             * 
             * \param nodeValueRequirement the NodeValueRequirement to add to the requirements.
             */
            void addValueRequirements(const NodeValueRequirement& nodeValueRequirement);

            /**
             * @brief Get the vector of NodeValueRequirement of the requirements.
             * 
             * \return the vector of NodeValueRequirement of the requirements.
             */
            const std::vector<NodeValueRequirement>& getValueRequirements() const;

            /**
             * \brief Get the NodeValuteRequirements at specified index
             * 
             * \param[in] idxValue specified index
             */
            const NodeValueRequirement& getValueRequirementsAt(size_t idxValue) const;

            /**
             * \brief get the number of valueRequirements
             */
            size_t size() const;
    };
}

#endif // NODE_REQUIREMENTS_H