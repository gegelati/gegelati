
#ifndef GP_NODE_H
#define GP_NODE_H

#include <vector>
#include <stdexcept>
#include <variant>
#include <functional>
#include <memory>

#include "data/dataValue.h"

struct CounterReset;

namespace Node {

    /**
     * \brief Class representing a GPNode
     * 
     * A node used to represent a GP genotype.
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
            std::vector<Data::DataValue> values;

            /// @brief Indicate wether the node is intron or not, default value is false.
            bool isIntron;

        public:
        
            /// Default polymorphic destructor
            virtual ~GPNode() = default;

            /// Disable copying to avoid accidental copies (use references or pointers instead).
            GPNode(const GPNode&) = delete;
            GPNode& operator=(const GPNode&) = delete;


            /**
             * \brief clone the current node as a unique pointer.
             */
            virtual std::unique_ptr<GPNode> cloneUniquePtr() const;

            /**
             * \brief Default empty constructor
             * 
             * \param[in] isIntron wether the node is intron or not, default value is false.
             */
            GPNode(bool isIntron = false):  isIntron{isIntron}, gpNodeID(incrementeCounter()) {};

            /**
             * \brief GPNode constructor.
             *  
             * \param[in] valuesRaw the values of the GPNode.
             * \param[in] isIntron wether the node is intron or not, default value is false.
             */
            GPNode(const std::vector<Data::DataValue>& valuesRaw, bool isIntron = false)
            : GPNode(isIntron) {
                for(const Data::DataValue& valueRaw: valuesRaw) {
                    this->addValue(valueRaw.clone());
                }};

            /**
             * \brief GPNode typed constructor.
             * Create a GPNode with uniform type T.
             * 
             * \tparam T Element type used the node values.
             * \param[in] valuesTyped the integer values of the GPNode.
             * \param[in] isIntron wether the node is intron or not, default value is false.
             */
            template <typename T>
            GPNode(const std::vector<T>& valuesTyped, bool isIntron = false)
            : GPNode(isIntron) {
                for(const T& valueTyped: valuesTyped) {
                    this->addValue(Data::DataValue::scalar<T>(valueTyped));
                }
            };

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
             * \brief add new DataValue at the end of the node
             * 
             * \param[in] value the value to set.
             */
            void addValue(const Data::DataValue& value);

            /**
             * \brief Set a DataValue at specified index
             * 
             * \param[in] index the index of the value to get.
             * \param[in] value the value to set.
             */
            void setValue(size_t index, const Data::DataValue& value);

            /**
             * \brief Set a scalar value of type T at specified index
             * 
             * \tparam T Element type used the node values.
             * \param[in] index the index of the value to get.
             * \param[in] value the value to set.
             */
            template <typename T>
            void setValue(size_t index, const T& value) {
                this->setValue(index, Data::DataValue::scalar<T>(value));
            }

            /**
             * \brief Get the value of the GPNode at the given index.
             * 
             * \param[in] index the index of the value to get.
             */
            const Data::DataValue& getValue(size_t index) const;

            /**
             * \brief set if the node is intron or not.
             * 
             * \param[in] isIntron value of statement.
             */
            void setIsIntron(bool isIntron);

            /**
             * \brief return if the node is intron or not.
             */
            bool getIsIntron() const;

            /**
             * \brief Get the number of values of the GPNode.
             */
            size_t getSize() const;

            /**
             * \brief Get the values of the GPNode.
             */
            const std::vector<Data::DataValue>& getValues() const;
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