#ifndef GRAPH_BASED_NODE_GENERATOR_H
#define GRAPH_BASED_NODE_GENERATOR_H

#include <vector>
#include "dimensions/constraint.h"
#include "dimensions/dataValueGenerator.h"

 namespace GraphBased {

    /**
     * \brief class representing the generator of a GPNode.
     *
     * The class is essentially composed of a vector of generators, one per value of the node.
     */
    class NodeGenerator {

        protected:
            /// @brief Vector of generators for the node.
            std::vector<std::unique_ptr<Dimensions::DataValueGenerator>> generators;
        public:

            NodeGenerator() {};

            // Disable copying to avoid accidental copies (use references or pointers instead).
            NodeGenerator(const NodeGenerator&) = delete;
            NodeGenerator& operator=(const NodeGenerator&) = delete;


            /**
             * @brief Add the generator of the next value of the node.
             *
             * \param generator the generator added.
             */
            void addGenerator(std::unique_ptr<Dimensions::DataValueGenerator> generator);
            /**
             * @brief Add the generator of the next value of the node.
             *
             * \param generator the generator added.
             */
            void addGenerator(const Dimensions::DataValueGenerator& generator);

            /**
             * \brief Get the DataValueGenerator at specified index
             * 
             * \param[in] idxValue specified index
             */
            const Dimensions::DataValueGenerator& getGeneratorAt(size_t idxValue) const;

            /**
             * \brief get the number of Generator
             */
            size_t size() const;

            /**
             * \brief Clone method duplicating the generators.
             */
            std::unique_ptr<NodeGenerator> cloneUniquePtr() const;
    };
}

#endif // GRAPH_BASED_NODE_GENERATOR_H