#ifndef NODE_TEMPLATE_H
#define NODE_TEMPLATE_H

#include "node/nodeValueTemplate.h"

namespace Node {

    class NodeTemplate {

        protected:
            /// @brief Vector of NodeValueTemplate for the node.
            std::vector<NodeValueTemplate> nodeValueTemplates;
        public:
            /**
             * @brief Default template constructor
             */
            NodeTemplate() {};

            /**
             * @brief Constructor with a vector of NodeValueTemplate
             * 
             * \param[in] nodeValueTemplates vector of NodeValueTemplate to initialize the NodeTemplate
             */
            NodeTemplate(const std::vector<NodeValueTemplate>& nodeValueTemplates) : nodeValueTemplates{nodeValueTemplates} {};

            /**
             * @brief Add a new NodeValueTemplate to the template.
             * 
             * \param nodeValueTemplate the NodeValueTemplate to add to the template.
             */
            void addValueTemplate(const NodeValueTemplate& nodeValueTemplate);

            /**
             * @brief Get the vector of NodeValueTemplate of the template.
             * 
             * \return the vector of NodeValueTemplate of the template.
             */
            const std::vector<NodeValueTemplate>& getValueTemplates() const;

            /**
             * \brief get the number of valueTemplates
             */
            size_t size() const;
    };
}

#endif // NODE_TEMPLATE_H