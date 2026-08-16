#ifndef GENOTYPE_H
#define GENOTYPE_H

#include "node/nodeGroup.h"

namespace Evolution {

    /**
     * \brief Class representing the genotype of a GP individual.
     * 
     * The genotype is characterized by nodes.
     * These nodes are stored inside a vector of NodeGroup, which is a vector of nodes.
     * The use of a vector NodeGroups allows different structure of nodes to coexist with various sizes.
     * 
     * For example, an LGP with 5 to 10 instructions, and 3 constants can be define with a first NodeGroup of 5 to 10 instruction nodes, and with a second NodeGroup refering the constants weights nodes.
     * 
     * Storing into vector of vector allow less confusions if a representation uses various number of nodes for multiple types of nodes.
     */
    class Genotype {

        protected:
            /// @brief Vector of node groups characterizing the genotype.
            std::vector<std::unique_ptr<Node::NodeGroup>> nodeGroups;

        public:

            // Disable copying to avoid accidental copies (use references or pointers instead).
            Genotype(const Genotype&) = delete;
            Genotype& operator=(const Genotype&) = delete;

            /// Default constructor
            Genotype() {};

            /**
             * \brief Add a NodeGroup to the Genotype.
             * 
             * \param[in] index the index at which to add the NodeGroup.
             */
            virtual Node::NodeGroup& addNodeGroup(size_t index);

            /**
             * \brief Add a NodeGroup at the end of the Genotype.
             */
            virtual Node::NodeGroup& addNodeGroup();

            /**
             * \brief Remove a NodeGroup from the Genotype.
             * 
             * \param[in] index the index at which to remove the NodeGroup.
             */
            virtual void removeNodeGroup(size_t index);

            /**
             * \brief Get the NodeGroup at the given index.
             * 
             * \param[in] index the index of the NodeGroup to get.
             */
            virtual const Node::NodeGroup& getNodeGroup(size_t index) const;

            /**
             * \brief Get the mutable NodeGroup at the given index.
             * 
             * \param[in] index the index of the NodeGroup to get.
             * 
             * \return a reference to the NodeGroup at the given index.
             */
            virtual Node::NodeGroup& getMutableNodeGroup(size_t index);

            /**
             * \brief Get the number of NodeGroup in the Genotype.
             */
            virtual size_t getSize() const;

            /**
             * \brief Get the number of nodes in the genotype.
             * 
             * \return the sum of the size of each NodeGroup.
             */
            virtual size_t getFullSize() const;

            /**
             * \brief Get the number of nodes in each NodeGroup.
             */
            virtual std::vector<size_t> getSizes() const;

            /**
             * \brief Get the NodeGroups of the Individual.
             * 
             * \return a reference to the vector of unique pointers to NodeGroup representing the genotype of the Individual.
             */
            virtual std::vector<std::reference_wrapper<const Node::NodeGroup>> getNodeGroups() const;

            /**
             * \brief Get the effective genotype of the Individual: the non-intron GPNodes.
             * 
             * \return a vector of vector for each non-intron node of each node group of the genotype.
             */
            virtual std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> getEffectiveNodes() const;

    };
    
    /**
     * Override of equality operator
     */
    bool operator==(const Genotype& genotype1, const Genotype& genotype2);

    /**
     * Override of inequality operator
     */
    bool operator!=(const Genotype& genotype1, const Genotype& genotype2);
};

#endif