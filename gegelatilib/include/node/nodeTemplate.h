#ifndef NODE_TEMPLATE_H
#define NODE_TEMPLATE_H

#include "node/nodeValueTemplate.h"

namespace Node {

    /**
     * \brief class representation the template of a GPNode.
     * 
     * The class is essentially composed on a vector of nodeValueTemplate, each one indicating the template of a value in the node.
     */
    class NodeTemplate {

        protected:
            /// @brief Vector of NodeValueTemplate for the node.
            std::vector<std::shared_ptr<const NodeValueTemplate>> nodeValueTemplates;
        public:
            /**
             * @brief Default template constructor
             */
            NodeTemplate() {};

            /**
             * @brief Constructor with a vector of NodeValueTemplate
             * 
             * \param[in] nodeValueTemplate single NodeValueTemplate to initialize the NodeTemplate
             */
            NodeTemplate(std::shared_ptr<const NodeValueTemplate> nodeValueTemplate) : nodeValueTemplates{nodeValueTemplate} {};

            /**
             * @brief Constructor with a vector of NodeValueTemplate
             * 
             * \param[in] nodeValueTemplates vector of NodeValueTemplate to initialize the NodeTemplate
             */
            NodeTemplate(const std::vector<std::shared_ptr<const NodeValueTemplate>>& nodeValueTemplates) : nodeValueTemplates{nodeValueTemplates} {};

            /**
             * @brief Add a new NodeValueTemplate to the template.
             * 
             * \param nodeValueTemplate the NodeValueTemplate to add to the template.
             */
            void addValueTemplate(std::shared_ptr<const NodeValueTemplate> nodeValueTemplate);

            /**
             * @brief Get the vector of NodeValueTemplate of the template.
             * 
             * \return the vector of NodeValueTemplate of the template.
             */
            const std::vector<std::shared_ptr<const NodeValueTemplate>>& getValueTemplates() const;

            /**
             * \brief Get the NodeValuteTemplate at specified index
             * 
             * \param[in] idxValue specified index
             */
            std::shared_ptr<const NodeValueTemplate> getValueTemplateAt(size_t idxValue) const;

            /**
             * \brief get the number of valueTemplates
             */
            size_t size() const;
    };
}

#endif // NODE_TEMPLATE_H