#ifndef NODE_GENERATOR_H
#define NODE_GENERATOR_H

#include <vector>
#include "dimensions/constraint.h"
#include "dimensions/dataValueGenerator.h"

namespace Node {

    /**
     * \brief class representation the generator of a GPNode.
     * 
     * The class is essentially composed on a vector of constraints and a vector of generators.
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
             * @brief Add a new constraint and generator to the generator.
             * 
             * \param generator the generator to add to the generator.
             */
            void addGenerator(std::unique_ptr<Dimensions::DataValueGenerator> generator);
            /**
             * @brief Add a new constraint and generator to the generator.
             * 
             * \param generator the generator to add to the generator.
             */
            void addGenerator(const Dimensions::DataValueGenerator& generator);

            /**
             * \brief Get the DataValueGenerator at specified index
             * 
             * \param[in] idxValue specified index
             */
            Dimensions::DataValueGenerator& getGeneratorAt(size_t idxValue);

            /**
             * \brief get the number of Generator
             */
            size_t size() const;

            /**
             * \brief Clone method returning the same shared pointer of constraints and dupplicated unique pointer of generators
             */
            std::unique_ptr<NodeGenerator> cloneUniquePtr() const;
    };
}

#endif // NODE_GENERATOR_H