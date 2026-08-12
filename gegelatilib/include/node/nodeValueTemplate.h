#ifndef NODE_VALUE_TEMPLATE_H
#define NODE_VALUE_TEMPLATE_H

#include "node/gpNode.h"

namespace Node {

    using NodeValueRange = std::variant<
        std::pair<size_t, size_t>,
        std::pair<double, double>
    >;

    using NodeValueConfiguration = std::variant<
        NodeValueRange,
        std::vector<NodeValue>
    >;


    /**
     * \brief Class representing the template of the value of a GPNode.
     * 
     * The template is composed of either
     *  - NodeValueRange: ranges of size_t or double,
     *  - nodeValues: list of possible values,
     * Stored in NodeValueConfiguration type
     * 
     * The template can stores multiple nodeValues and multiple NodeValueRange.
     * The value can take any of the value in the proposed NodeValueRange, or any of the nodeValue in the possible nodeValues.
     */
    class NodeValueTemplate {
        protected:
            /// @brief Vector of possible configurations for the nodeValue.
            std::vector<NodeValueConfiguration> configurations;

        public:

            /**
             * @brief Default constructor
             */
            NodeValueTemplate() {};

            /**
             * @brief Constructor with a vector of NodeValueConfiguration
             * 
             * @param configurations vector of NodeValueConfiguration to initialize the NodeValueTemplate
             */
            NodeValueTemplate(const std::vector<NodeValueConfiguration>& configurations) : configurations{configurations} {};

            /**
             * @brief Add a new configuration to the template.
             * 
             * @param configuration the configuration to add to the template.
             */
            void addValueConfiguration(const NodeValueConfiguration& configuration);

            /**
             * @brief Get the vector of NodeValueConfiguration of the template.
             * 
             * @return the vector of NodeValueConfiguration of the template.
             */
            const std::vector<NodeValueConfiguration>& getconfigurations() const;

            /**
             * @brief Get the NodeValueConfiguration of the template.
             * 
             * \param[in] idxCondif index of the required configuration
             */
            const NodeValueConfiguration& getconfigurationAt(size_t idxConfig) const;

            /**
             * \brief get the size of the NodeValueTemplate. 
             * The number of configuration.
             */
            size_t size() const;
    };
}

#endif // NODE_VALUE_TEMPLATE_H