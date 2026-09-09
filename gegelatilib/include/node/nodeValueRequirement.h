#ifndef NODE_VALUE_TEMPLATE_H
#define NODE_VALUE_TEMPLATE_H

#include "dimensions/requirement.h"
#include "node/gpNode.h"

namespace Node {


    /**
     * \brief Class representing the requirements of the value of a GPNode.
     * 
     * The class can stores multiple requirements.
     * The value can take any of the value in the proposed requirements.
     */
    class NodeValueRequirement {
        protected:
            /// @brief Vector of possible requirements for the nodeValue.
            std::vector<Dimensions::Requirement> requirements;

        public:

            /**
             * @brief Default constructor
             */
            NodeValueRequirement() {};

            /**
             * @brief Constructor with a vector of Requirement
             * 
             * @param requirement single Requirement to initialize the NodeValueRequirement
             */
            NodeValueRequirement(const Dimensions::Requirement& requirement) : requirements{requirement} {};

            /**
             * @brief Constructor with a vector of Requirement
             * 
             * @param requirements vector of Requirement to initialize the NodeValueRequirement
             */
            NodeValueRequirement(const std::vector<Dimensions::Requirement>& requirements) : requirements{requirements} {};

            /**
             * @brief Add a new requirement to the template.
             * 
             * @param requirement the requirement to add to the template.
             */
            void addRequirement(const Dimensions::Requirement& requirement);

            /**
             * @brief Get the vector of Requirement of the template.
             * 
             * @return the vector of Requirement of the template.
             */
            const std::vector<Dimensions::Requirement>& getRequirements() const;

            /**
             * @brief Get the Requirement of the template.
             * 
             * \param[in] idxConfig index of the required requirement
             */
            const Dimensions::Requirement& getRequirementAt(size_t idxConfig) const;

            /**
             * \brief get the size of the NodeValueRequirement. 
             * The number of requirement.
             */
            size_t size() const;
    };
}

#endif // NODE_VALUE_TEMPLATE_H