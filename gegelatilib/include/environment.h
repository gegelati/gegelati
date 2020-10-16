/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cmath>
#include <iostream>

#include "data/constantHandler.h"
#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "instructions/instruction.h"
#include "instructions/set.h"

/// LineSize structure to be used within the Environment.
typedef struct LineSize
{
    /// Number of bits used to encode the instructionIndex
    size_t nbInstructionBits;
    /// Number of bits used to encode the destinationIndex
    size_t nbDestinationBits;
    /// Total number of bits used to encode the operands info
    size_t nbOperandsBits;
    /// Number of bits used for each operand pair, to encode dataSourceIndex
    size_t nbOperandDataSourceIndexBits;
    /// Number of bits used for each operand pair, to encode location
    size_t nbOperandLocationBits;
    /// Total number of bits to encode a program line.
    size_t totalNbBits;

    /// Default cast to uint64_t returns the total number of bits.
    operator size_t() const
    {
        return totalNbBits;
    }
} LineSize;

/**
 * \brief The Environment class contains all information needed to execute a
 * Program.
 *
 * To execute a Program, and size adequately its ProgramLine, a fixed
 * Instruction Set, a list of available input DataHandler, and the number of
 * available registers is needed.
 *
 * To ensure viability of Program based on a given Environment, all attributes
 * of an Environment are copied in const attributes at construction time.
 */
class Environment
{
  protected:
    /// Set of Instruction used by Program running within this Environment.
    const Instructions::Set instructionSet;

    /// List of DataHandler that can be accessed within this Environment.
    const std::vector<std::reference_wrapper<const Data::DataHandler>>
        dataSources;

    /// Number of registers
    const size_t nbRegisters;

    /// Number of constants
    const size_t nbConstants;

    /// Vector of DataHandlers containing the environment's dataSources
    std::vector<std::reference_wrapper<const Data::DataHandler>>
        fakeDataSources;

    /// DataHandler whost type corresponds to registers.
    const Data::PrimitiveTypeArray<double> fakeRegisters;

    /// DataHandler whost type corresponds to the programs constants.
    const Data::ConstantHandler fakeConstants;

    /// Number of Instruction in the Instructions::Set.
    const size_t nbInstructions;

    /// Maxmimum number of operands of the Instructions::Set.
    const size_t maxNbOperands;

    /// Number of DataHandler from which data can be accessed.
    const size_t nbDataSources;

    /// Size of the largestAddressSpace of DataHandlers
    const size_t largestAddressSpace;

    /// Size of lines within this Environment
    const LineSize lineSize;

    /**
     * \brief Static method used when constructing a new Environment to compute
     * the largest AddressSpace of a set of DataHandler.
     *
     * \param[in] nbRegisters the number of registers of the environment.
     * \param[in] nbConstants the number of program's constants.
     * \param[in] dHandlers reference to the set of DataHandler whose largest
     * largestAddressSpace is searched. \return the found value, or 0 default
     * value if the given std::vector was empty.
     */
    static size_t computeLargestAddressSpace(
        const size_t nbRegisters, const size_t nbConstants,
        const std::vector<std::reference_wrapper<const Data::DataHandler>>&
            dHandlers);

    /**
     * \brief Static method used to compute the size of Program lines based on
     * information from the Enviroment.
     *
     * The Program line size, expressed in bits, is computed with the following
     * formula: $ ceil(log2(i)) + ceil(log2(n))+
     * m*(ceil(log2(nb_{src}))+ceil(log2(largestAddressSpace)) + p*32$ With bits
     * organised (theoretically) in the same order as in the equation |
     * Instruction | destination | operands See
     * PROJECT/doc/instructions.md for more details.
     *
     * \param[in] env The Environment whose information is used.
     * \return the computed line size.
     * \throw std::domain_error in cases where the given Environment is
     * parameterized with no registers, contains no Instruction, Instruction
     * with no operands, no DataHandler or DataHandler with no addressable
     * Space.
     */
    static const LineSize computeLineSize(const Environment& env);

    /**
     * \brief Filter an InstructionSet to keep only Instruction with operand
     * types provided by the given DataHandler.
     *
     * \param[in] iSet the Instructions::Set to filter.
     * \param[in] nbRegisters Number of registers
     * \param[in] nbConstants Number of constants of the program
     * \param[in] dataSources a set of DataHandler providing data.
     * \return a new Instructions:Set where only Instruction whose operands
     * can be provided by at least one DataHandler are kept.
     */
    static Instructions::Set filterInstructionSet(
        const Instructions::Set& iSet, const size_t nbRegisters,
        const size_t nbConstants,
        const std::vector<std::reference_wrapper<const Data::DataHandler>>&
            dataSources);

  private:
    /// Default constructor deleted for its uselessness.
    Environment() = delete;

  public:
    /**
     * \brief Constructor with initialization of all attributes.
     *
     * To ensure viability of Program based on a given Environment, all
     * attributes of an Environment are copied in const attributes at
     * construction time.
     *
     * \param[in] iSet the Instructions::Set whose Instruction will be used in
     * this Environment.
     * \param[in] dHandlers the list of DataHandler that will
     * be used in this Environment.
     * \param[in] nbRegs the number of double registers in this Environment.
     * \param[in] nbConst the number of program's constants in this Environment.
     */
    Environment(
        const Instructions::Set& iSet,
        const std::vector<std::reference_wrapper<const Data::DataHandler>>&
            dHandlers,
        const size_t nbRegs, const size_t nbConst = 0)
        : instructionSet{filterInstructionSet(iSet, nbRegs, nbConst,
                                              dHandlers)},
          dataSources{dHandlers}, nbRegisters{nbRegs}, nbConstants{nbConst},
          fakeRegisters(nbRegs), fakeConstants(nbConst),
          nbInstructions{instructionSet.getNbInstructions()},
          maxNbOperands{instructionSet.getMaxNbOperands()},
          nbDataSources{
              dHandlers.size() +
              (nbConst > 0 ? 2
                           : 1)}, // if Constants are used, we need an extra
                                  // datasource to store them in the environment
          largestAddressSpace{
              computeLargestAddressSpace(nbRegs, nbConst, dHandlers)},
          lineSize{computeLineSize(*this)}
    {
        this->fakeDataSources.push_back(
            (std::reference_wrapper<const Data::DataHandler>)this
                ->fakeRegisters);

        if (nbConst > 0) {
            this->fakeDataSources.push_back(this->fakeConstants);
        }

        for (auto& elem : this->dataSources)
            this->fakeDataSources.push_back(elem);
    };

    /**
     * \brief Get the size of the number of registers of this Environment.
     *
     * \return the value of the nbRegisters attribute.
     */
    size_t getNbRegisters() const;

    /**
     * \brief Get the number of constants used by programs.
     *
     * \return the value of the nbParameters attribute.
     */
    size_t getNbConstant() const;

    /**
     * \brief Get the size of the number of Instruction within the
     * Instructions::Set.
     *
     * \return the value of the nbInstructions attribute.
     */
    size_t getNbInstructions() const;

    /**
     * \brief Get the size of the maximum number of operands of
     * Instructions::Set.
     *
     * \return the value of the maxNbOperands attribute.
     */
    size_t getMaxNbOperands() const;

    /**
     * \brief Get the size of the number of DataHandlers.
     *
     * \return the value of the nbDataSources attribute.
     */
    size_t getNbDataSources() const;

    /**
     * \brief Get the size of the largestAddressSpace of DataHandlers.
     *
     * \return the value of the LargestAddressSpace attribute.
     */
    size_t getLargestAddressSpace() const;

    /**
     * \brief Get the size of the line for this environment (in bits).
     *
     * \return the value of the lineSize attribute.
     */
    const LineSize& getLineSize() const;

    /**
     * \brief Get the DataHandler of the Environment.
     *
     * \return a const reference to the dataSources attribute of this
     * Environment.
     */
    const std::vector<std::reference_wrapper<const Data::DataHandler>>&
    getDataSources() const;

    /**
     * Get the datasource identical to the one used by programs
     *
     * Getting the data sources identical to the one used by programs
     * when executing a Program can be useful, notably when
     * mutating a Program::Line and assessing whether a data type can be
     * provided by the registers.
     */
    const std::vector<std::reference_wrapper<const Data::DataHandler>>&
    getFakeDataSources() const;

    /**
     * \brief Get the Instruction Set of the Environment.
     *
     * \return a const reference to the instructionSet attribute of this
     * Environment.
     */
    const Instructions::Set& getInstructionSet() const;
};

#endif
