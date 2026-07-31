
#ifndef LGP_AGENT_H
#define LGP_AGENT_H

#include <cstdint>
#include "representation/individual.h"
#include "evoGraph/vertex.h"
#include "representation/lgp/lgpLine.h"
#include "representation/lgp/environment.h"
#include "data/constantHandler.h"
#include "outputInfo.h"

namespace Representation::LGP {
      
        /**
         * \brief Abstract class representing a LgpIndividual
         */
        class LgpIndividual : public Individual
        {
        protected:

            /// Environment within which the LGP will be executed.
            const LGPEnvironment& environment;
            
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
            std::vector<std::pair<std::unique_ptr<LGPLine>, bool>> lines;


            /**
             *   \brief Constants of the LGP
             *
             *   A LGP contains a set of constants in a dedicated
             *	 Data::DataHandler
            **/
            Data::ConstantHandler constants;

            /// Number of outputs of the LGP
            const Output::OutputHandler& outputs;

            /// Output indices
            std::vector<size_t> outputIndices;

            /// Delete the default constructor.
            LgpIndividual() = delete;

        public:
            /**
             * \brief Main constructor of the LgpIndividual.
             *
             * \param[in] e the reference to the Environment that will be referenced
             * by the LgpIndividual.
             * \param[in] outputs outputs of the LGP.
             * \param[in] representationID id of the representation used.
             */
            LgpIndividual(const LGPEnvironment& e, const Output::OutputHandler& outputs, uint64_t representationID)
                : Individual(representationID), environment{e}, constants{e.getNbConstants()}, outputs{outputs}
            {
                constants.resetData(); // force all constant to 0 at first.
                size_t nbOutputs = this->getUsedNbOutputs(outputs);
                for (size_t idx = 0; idx < nbOutputs; idx++ ){
                    outputIndices.push_back(idx % e.getNbRegisters());
                }
            };

            /**
             * \brief get real number of outputs
             */
            size_t getUsedNbOutputs(const Output::OutputHandler& outputs) const;

            // Disable copy semantics: LgpIndividual should never be copied.
            LgpIndividual(const LgpIndividual&) = delete;
            LgpIndividual& operator=(const LgpIndividual&) = delete;

            
            /**
             * \brief Destructor for the LgpIndividual class.
             *
             * This destructor deallocates all memory allocated for LgpIndividual lines
             * (if any).
             */
            ~LgpIndividual();

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
            const LGPLine& addNewLine();

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
            const LGPLine& addNewLine(uint64_t idx);

            /**
             * \brief Add a new line to the LGP at the end of the LGP.
             *
             * \param[in] newLine new LGPLine copied to the LGP
             * \param[in] idx the position at which the line should be inserted.
             */
            void addNewLine(const LGPLine& newLine, uint64_t idx);

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
            const LGPEnvironment& getEnvironment() const;

            /**
             * \brief Get the number of lines in the LGP.
             *
             * \return the number of line contained in the PrograLGPm.
             */
            size_t getNbLines() const;

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
             * \brief Get a ref to a LGPLine of the LGP for mutation.
             * 
             * \param[in] index The integer index of the retrieved LGPLine within the
             * LGP.              
             * \return a reference to the indexed LGPLine of the LGP.
             */
            LGPLine& getLineForMutation(uint64_t index);

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
             * \brief getter for outputIndices
             */
            const std::vector<size_t>& getOutputIndices() const;

            /**
             * \brief set a new output index at a specific location.
             * 
             * \param[in] newOutputIndex new output index set
             * \param[in] location location of input at which the new index is set
             */
            void setOutputIndex(size_t newOutputIndex, size_t location);

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
    
}; // namespace Representation::LGP

#endif // LGP_AGENT
