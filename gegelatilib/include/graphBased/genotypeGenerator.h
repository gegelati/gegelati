
#ifndef GRAPH_BASED_GENOTYPE_GENERATOR_H
#define GRAPH_BASED_GENOTYPE_GENERATOR_H

#include "graphBased/genotypeConstraint.h"
#include "graphBased/nodeGenerator.h"

 namespace GraphBased {


    /**
     * \brief class representing the generator of a genotype for a representation.
     *
     * The GenotypeGenerator is built from the constraint of a representation (or by hand), and used to create or modify a genotype.
     * 
     * The GenotypeGenerator stores multiple NodeGenerator with corresponding ranges.
     * For example, a genotype with 
     *  - nodeGenerators = {generator0, generator1},
     *  - ranges = {{1, 5}, {3}},
     * can create individuals with 1 to 5 nodes corresponding to the "generator0" and 3 nodes corresponding to the "generator1".
     */
    class GenotypeGenerator {



        protected:

            /// @brief The vector of node generator.
            std::vector<std::unique_ptr<NodeGenerator>> nodeGenerators;

            /// @brief Range of the number of nodes of each group during the mutations.
            std::vector<std::pair<size_t, size_t>> nodeMutationRanges;
            /// @brief Range of the number of nodes of each group at the initialization.
            std::vector<std::pair<size_t, size_t>> nodeInitializeRanges;
        public:
            /**
             * \brief Default Constructor.
             */
            GenotypeGenerator() {};

            // Disable copying to avoid accidental copies (use references or pointers instead).
            GenotypeGenerator(const GenotypeGenerator&) = delete;
            GenotypeGenerator& operator=(const GenotypeGenerator&) = delete;


            /**
             * \brief Constructor with one generator and range (for quick construction)
             * 
             * \param[in] nodeGenerator single NodeGenerator to initialize the GenotypeGenerator
             * \param[in] mutationRange ranges of node allowed during mutation.
             * \param[in] initRange ranges of node allowed during initialization. default 0 set the value of mutation Range
             */
            GenotypeGenerator(const NodeGenerator& nodeGenerator, std::pair<size_t, size_t> mutationRange, std::pair<size_t, size_t>  initRange = {0, 0}
            ) : nodeMutationRanges{mutationRange} {
                if(initRange.first == 0 && initRange.second == 0) {
                    this->nodeInitializeRanges.push_back(mutationRange);
                } else {
                    this->nodeInitializeRanges.push_back(initRange);
                }
                this->nodeGenerators.push_back(std::move(nodeGenerator.cloneUniquePtr()));
            };

            /**
             * \brief Create the naive generator of a constraint: each value is sampled uniformly in its constraint, each group has the range of the constraint.
             *
             * \param[in] constraint the constraint of the genotypes.
             * \throw std::runtime_error if a value has a constraint without naive generator (see Dimensions::Constraint::makeGenerator).
             */
            explicit GenotypeGenerator(const GenotypeConstraint& constraint);

            /**
             * \brief return the number of node generator.
             */
            size_t size() const;

            /**
             * \brief add a node generator with its range.
             * 
             * \param[in] nodeGenerator node generator added
             * \param[in] mutationRange ranges of node allowed during mutation.
             * \param[in] initRange ranges of node allowed during initialization. default 0 set the value of mutation Range
             */
            void addNodeGenerator(const NodeGenerator& nodeGenerator, std::pair<size_t, size_t> mutationRange, std::pair<size_t, size_t>  initRange = {0, 0});

            /**
             * \brief add a node generator with its range.
             * 
             * \param[in] nodeGenerator unique_ptr of node generator added
             * \param[in] mutationRange ranges of node allowed during mutation.
             * \param[in] initRange ranges of node allowed during initialization. default 0 set the value of mutation Range
             */
            void addNodeGenerator(std::unique_ptr<NodeGenerator> nodeGenerator, std::pair<size_t, size_t> mutationRange, std::pair<size_t, size_t>  initRange = {0, 0});

            /**
             * \brief add a node generator with its range.
             * 
             * \param[in] nodeGenerator node generator added
             * \param[in] fixRange fixed range of value
             */
            void addFixRangeNodeGenerator(const NodeGenerator& nodeGenerator, size_t fixRange);

            /**
             * \brief add a node generator with its range.
             * 
             * \param[in] nodeGenerator unique_ptr of node generator added
             * \param[in] fixRange fixed range of value
             */
            void addFixRangeNodeGenerator(std::unique_ptr<NodeGenerator> nodeGenerator, size_t fixRange);

            /**
             * \brief Return the node generator at specified index.
             * 
             * \param[in] idx specified index
             */
            const NodeGenerator& getNodeGeneratorAt(size_t idx) const;

            /**
             * \brief Return the mutation range at specified index.
             * 
             * \param[in] idx specified index
             */
            const std::pair<size_t, size_t>& getMutationRangeAt(size_t idx) const;


            /**
             * \brief Return the initialization range at specified index.
             * 
             * \param[in] idx specified index
             */
            const std::pair<size_t, size_t>& getInitRangeAt(size_t idx) const;

            /**
             * \brief return a copy of the current GenotypeGenerator by dupplicating each NodeGenerator and range
             */
            std::unique_ptr<GenotypeGenerator> cloneUniquePtr() const;
    };
}

#endif // GRAPH_BASED_GENOTYPE_GENERATOR_H