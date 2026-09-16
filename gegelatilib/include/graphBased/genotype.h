#ifndef GRAPH_BASED_GENOTYPE_H
#define GRAPH_BASED_GENOTYPE_H

#include "graphBased/nodeGroup.h"

 namespace GraphBased {

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
            std::vector<std::unique_ptr<NodeGroup>> nodeGroups;

        public:

            // Disable copying to avoid accidental copies (use references or pointers instead).
            Genotype(const Genotype&) = delete;
            Genotype& operator=(const Genotype&) = delete;

            /// Default constructor
            Genotype() {};

            /**
             * \brief Method for fast construction of simple genotypes
             * 
             * \param[in] node single node of the genotype
             */
            static std::unique_ptr<Genotype> singleNodeGenotype(std::unique_ptr<GPNode> node);

            /**
             * \brief Method for fast construction of simple genotypes
             * 
             * \param[in] group single group of the genotype
             */
            static std::unique_ptr<Genotype> singleGroupGenotype(std::unique_ptr<NodeGroup> group);

            /**
             * \brief clone the current genotype as a unique pointer.
             */
            virtual std::unique_ptr<Genotype> cloneUniquePtr() const;

            /**
             * \brief Add a NodeGroup to the Genotype.
             * 
             * \param[in] group the group added.
             * \param[in] index the index at which to add the NodeGroup.
             */
            virtual void addNodeGroup(std::unique_ptr<NodeGroup> group, size_t index);

            /**
             * \brief Add a NodeGroup at the end of the Genotype.
             * 
             * \param[in] group the group added.
             */
            virtual void addNodeGroup(std::unique_ptr<NodeGroup> group);

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
            virtual const NodeGroup& getNodeGroup(size_t index) const;

            /**
             * \brief Get the NodeGroup at the given index.
             * 
             * \param[in] index the index of the NodeGroup to get.
             */
            virtual NodeGroup& getNodeGroup(size_t index);

            /**
             * \brief set a NodeGroup at the given index, replacing the former one.
             * 
             * \param[in] group the group set.
             * \param[in] index the index of the NodeGroup to set.
             */
            virtual void setNodeGroup(std::unique_ptr<NodeGroup> group, size_t index);

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
            virtual std::vector<std::reference_wrapper<const NodeGroup>> getNodeGroups() const;

            /**
             * \brief Get the effective genotype of the Individual: the non-intron GPNodes.
             * 
             * \return a vector of vector for each non-intron node of each node group of the genotype.
             */
            virtual std::vector<std::vector<std::reference_wrapper<const GPNode>>> getEffectiveNodes() const;

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

#endif // GRAPH_BASED_GENOTYPE_H