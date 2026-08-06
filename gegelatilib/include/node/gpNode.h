
#ifndef GP_NODE_H
#define GP_NODE_H

#include <vector>
#include <stdexcept>

struct CounterReset;

namespace Node {

    /**
     * \brief Class representing a GPNode
     * 
     * A hierarchical node used to represent a GP genotype.
     * GPNode are composed of values of size_t type, as well as max ranges for each value (min is 0).
     */
    class GPNode
    {
        private:

        
            /// Unique ID of the GPNode.
            size_t gpNodeID;

            /**
             * \brief Incremente the GPNode ID counter and return the new value.
             */
            static size_t incrementeCounter();

            /**
             * \brief Reset the GPNode ID counter.
             *
             * This method set the ID counter to a new value.
             * It can quickly lead to segmentation fault if not used carefully.
             */
            static void resetGPNodeIDCounter();
            friend struct ::CounterReset;

            /// \brief Values of the GPNode.
            std::vector<size_t> values;

            /// \brief Maximum ranges of the values of the GPNode.
            std::vector<size_t> max_ranges;

            /// \brief Size of the GPNode.
            size_t size;

        public:
        
            /// Default polymorphic destructor
            virtual ~GPNode() = default;

            /// Disable copying to avoid accidental copies (use references or pointers instead).
            GPNode(const GPNode&) = delete;
            GPNode& operator=(const GPNode&) = delete;

            /**
             * \brief GPNode constructor.
             *  
             * \param[in] values the values of the GPNode.
             * \param[in] max_ranges the maximum ranges of the values of the GPNode.
             */
            GPNode(const std::vector<size_t>& values, const std::vector<size_t>& max_ranges): values(values), max_ranges(max_ranges), size(max_ranges.size()), gpNodeID(incrementeCounter()) {
                if (values.size() != max_ranges.size()){
                    throw std::runtime_error("Node::GPNode: values and max_ranges must have the same size.");
                }
            };

            
            /**
             * \brief GPNode constructor.
             *  
             * \param[in] max_ranges the maximum ranges of the values of the GPNode.
             */
            GPNode(const std::vector<size_t>& max_ranges): GPNode(std::vector<size_t>(max_ranges.size(), 0), max_ranges){};


            /**
             * \brief return the ID of the GPNode.
             */
            static size_t getGPNodeIDCounter();
            
            /**
             * \brief Get the unique identifier of the GPNode.
             *
             * \return the integer ID of the GPNode.
             */
            virtual size_t getGPNodeID() const;

            /**
             * \brief Set a new unique identifier to the GPNode.
             *
             * \param[in] newID the new integer ID to set to the GPNode.
             */
            virtual void setGPNodeID(size_t newID);

            /**
             * \brief GPNode constructor.
             * 
             * \param[in] index the index of the value to get.
             * \param[in] value the value to set.
             */
            void setValue(size_t index, size_t value);

            /**
             * \brief Get the value of the GPNode at the given index.
             * 
             * \param[in] index the index of the value to get.
             */
            size_t getValue(size_t index) const;


            /**
             * \brief Get the max range of the GPNode at the given index.
             * 
             * \param[in] index the index of the max range to get.
             */
            size_t getMaxRange(size_t index) const;

            /**
             * \brief Get the number of values of the GPNode.
             */
            size_t getSize() const;

            /**
             * \brief Get the values of the GPNode.
             */
            const std::vector<size_t>& getValues() const;

            /**
             * \brief Get the maximum ranges of the GPNode.
             */
            const std::vector<size_t>& getMaxRanges() const;
    };

    
    /**
     * \brief Comparison function to enable sorting of GPNode with
     * STL.
     */
    bool operator<(const GPNode& a, const GPNode& b);

    /**
     * \brief Comparison function to enable sorting of GPNode with
     * STL.
     */
    bool operator==(const GPNode& a, const GPNode& b);

    /**
     * \brief Comparison function to enable sorting of GPNode with
     * STL.
     */
    bool operator!=(const GPNode& a, const GPNode& b);

}; // namespace Node

#endif // GP_NODE_H