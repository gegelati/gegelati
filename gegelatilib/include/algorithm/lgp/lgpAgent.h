
#ifndef LGP_AGENT_H
#define LGP_AGENT_H

#include <cstdint>
#include "algorithm/agent.h"
#include "evoGraph/vertex.h"
#include "algorithm/lgp/lgpLine.h"
#include "environment.h"
#include "data/constantHandler.h"
#include "outputInfo.h"

namespace Algorithm::LGP {
      
        /**
         * \brief Abstract class representing a LGPAgent
         */
        class LGPAgent : public Agent
        {
        protected:

            /// Environment within which the LGP will be executed.
            const std::shared_ptr<const Environment>& environment;
            
            /**
             * \brief Lines of the LGP and intron property.
             *
             * Each element of this vector stores a pointer to a LGPLine, and a
             * boolean value indicating whether this LGPLine is an Intron whithin the
             * LGP.
             *
             * Introns are Lines of the LGP that do not contribute to its final
             * result, stored in the first register. Hence, skipping these lines
             * during a LGP execution can speed up the LGP execution.
             */
            std::vector<std::pair<std::shared_ptr<LGPLine>, bool>> lines;


            /**
             *   \brief Constants of the LGP
             *
             *   A LGP contains a set of constants in a dedicated
             *	 Data::DataHandler
            **/
            Data::ConstantHandler constants;

            /// Number of outputs of the LGP
            const Output::OutputHandler& outputs;

            /// Delete the default constructor.
            LGPAgent() = delete;

        public:
            /**
             * \brief Main constructor of the LGPAgent.
             *
             * \param[in] e the reference to the Environment that will be referenced
             * by the LGPAgent.
             * \param[in] outputs outputs of the LGP.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] element the element on which the agent is set. Can be a nullptr if no elements are needed
             */
            LGPAgent(const std::shared_ptr<const Environment>& e, const Output::OutputHandler& outputs, std::string algorithmName, std::shared_ptr<const EvoGraph::Element> element = nullptr)
                : Agent(algorithmName, element), environment{e}, constants{e->getParams().nbProgramConstant}, outputs{outputs}
            {
                constants.resetData(); // force all constant to 0 at first.
            };

            
            /**
             * \brief Destructor for the LGPAgent class.
             *
             * This destructor deallocates all memory allocated for LGPAgent lines
             * (if any).
             */
            ~LGPAgent();

            /**
             * \brief Get the number of outputs of the LGP.
             *
             * \return the number of outputs of the LGP.
             */
            const Output::OutputHandler& getOutputs() const;

            
            /**
             * \brief Add a new line to the LGP with only 0 bits.
             *
             * The new line is inserted at the end of the LGP.
             *
             * \return a non-const reference to the newly added LGPLine.
             */
            LGPLine& addNewLine();

            /**
             * \brief Add a new line to the LGP with only 0 bits.
             *
             * The new line is inserted at the given position of the LGP.
             *
             * \param[in] idx the position at which the line should be inserted.
             * \return a non-const reference to the newly added LGPLine.
             * \throw std::out_of_range if the given position is beyond the end of
             * the LGP.
             */
            LGPLine& addNewLine(uint64_t idx);

            /**
             * \brief Add a new line to the LGP at the end of the LGP.
             *
             * \param[in] newLine new LGPLine copied to the LGP
             */
            void addNewLine(const LGPLine& newLine);

            /**
             * \brief Clear all intron instructions in the LGP.
             *
             * All introns lines from the LGP are removed by this method.
             * The behavior of the LGP should not be modified after a call
             * to this function.
             *
             * Introns should have been identified before calling this methos, as
             * this method does NOT call the identifyIntrons method.
             */
            void clearIntrons();

            /**
             * \brief Remove a LGPLine from the LGP.
             *
             * Remove the LGPLine at the given index from the LGP and free the
             * associated memory.
             *
             * \param[in] idx the integer index of the LGP LGPLine to remove.
             * \throw std::out_of_range if the index is too large.
             */
            void removeLine(const uint64_t idx);

            /**
             * \brief Swap two existing lines from the LGP.
             *
             * \param[in] idx0 the index of the first line to swap.
             * \param[in] idx1 the index of the second line to swap.
             * \throw std::out_of_range if any of the two index is too large.
             */
            void swapLines(const uint64_t idx0, const uint64_t idx1);

            /**
             * \brief Get the environment associated to the LGP at construction.
             *
             * \return a const reference to the Environment of the LGP and all
             * its LGPLine.
             */
            const std::shared_ptr<const Environment>& getEnvironment() const;

            /**
             * \brief Get the number of lines in the LGP.
             *
             * \return the number of line contained in the PrograLGPm.
             */
            size_t getNbLines() const;

            /**
             * \brief Get a const pointer to a LGPLine of the LGP.
             * 
             * \param[in] index The integer index of the retrieved LGPLine within the
             * LGP.
             * \return a const pointer to the indexed LGPLine of the
             * LGP.
             * 
             * \throw std::out_of_range if the index is too large.
             */
            std::shared_ptr<const LGPLine> getLinePtr(uint64_t index) const;

            /**
             * \brief Get a const ref to a LGPLine of the LGP.
             *
             * \param[in] index The integer index of the retrieved LGPLine within the
             * LGP. 
             * \return a const reference to the indexed LGPLine of the
             * LGP. 
             * \throw std::out_of_range if the index is too large.
             */
            const LGPLine& getLine(uint64_t index) const;

            /**
             * \brief Get a non-const ref to a LGPLine of the LGP.
             *
             * \param[in] index The integer index of the retrieved LGPLine within the
             * LGP. 
             * \return a const reference to the indexed LGPLine of the
             * LGP. 
             * \throw std::out_of_range if the index is too large.
             */
            LGPLine& getLine(uint64_t index);

            /**
             * \brief Checks whether a LGPLine at the given index is an intron.
             *
             * \param[in] index The integer index of the checked LGPLine within the
             * LGP.
             * \return true if the LGPLine is an intron, false otherwise.
             * \throw std::out_of_range if the index is too large.
             */
            bool isIntron(uint64_t index) const;

            /**
             * \brief Set if the specified line is an intron
             * 
             * \param[in] index The integer index of the LGPLine within the
             * LGP.
             * \param[in] isIntron boolean to set the intron value of the line.
             */
            void setIntronValue(uint64_t index, bool isIntron);

            /**
             *  \brief get the constantHandler object of the LGP
             *
             *  This method gives a reference to the constantHandler associated
             *  with the LGP
             *
             *  \return the constantHandler of the LGP
             */
            Data::ConstantHandler& getConstantHandler();

            /**
             *  \brief get a const reference to the constantHandler object of the
             * LGP
             *
             *  This method gives a const reference to the constantHandler
             * associated with the LGP
             *
             *  \return the constantHandler of the LGP through a const reference
             */
            const Data::ConstantHandler& cGetConstantHandler() const;

            /**
             *	\brief Get the value of a constant at a given index
            *
            *	Although this method is not required as the data is accessible from
            *	the constantHandler, it allows a shortcut and add readability.
            *
            * \param[in] index the position at which we access the constant
            *	\return the value of the constant at the given index
            */
            const Data::Constant getConstantAt(size_t index) const;

        };
    
}; // namespace Algorithm::LGP

#endif // LGP_AGENT
