/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#include <set>
#include <stdexcept>

#include "environment.h"
#include "mutator/lineMutator.h"

/**
 * \brief Function to initialize a single operand of a Program::Line.
 *
 * This function uses the Mutator::RNG to set (if possible) the nth operand
 * pair of a given Line to a valid value according to the provided Environment
 * and the selected Instruction.
 *
 * \param[in] instruction the selected Instruction for this line.
 * \param[in,out] line the Program::Line to initialize.
 * \param[in] operandIdx the index of the operand of the Line to initialize.
 * \param[in] initOperandDataSource should the operand data source be
 * (re-)initialized? \param[in] initOperandLocation should the operand location
 * be (re)initialized? \param[in] forceChange should the value be mandatorily
 * new? \param[in] rng Random Number Generator used in the mutation process.
 * \return true if the operand was successfully initialized, false if no valid
 * data source could be found for this Instruction and operandIdx couple.
 */
static bool initRandomCorrectLineOperand(
    const Instructions::Instruction& instruction, Program::Line& line,
    const uint64_t& operandIdx, const bool initOperandDataSource,
    const bool initOperandLocation, const bool forceChange, Mutator::RNG& rng)
{
    const Environment& env = line.getEnvironment();
    uint64_t operandDataSourceIndex = line.getOperand(operandIdx).first;
    bool operandFound = !initOperandDataSource;

    // Is the operand constrained in type?
    if (initOperandDataSource && operandIdx < instruction.getNbOperands()) {
        // Select an operand
        // The type of operand needed
        const std::type_info& operandType =
            instruction.getOperandTypes().at(operandIdx).get();

        // keep a record of tested indexes
        std::set<uint64_t> operandDataSourceIndexes;
        if (forceChange) {
            operandDataSourceIndexes.insert(operandDataSourceIndex);
        }

        while (!operandFound &&
               operandDataSourceIndexes.size() < env.getNbDataSources()) {
            // Select an operandDataSourceIndex
            operandDataSourceIndex =
                rng.getUnsignedInt64(0, (env.getNbDataSources() - 1) -
                                            operandDataSourceIndexes.size());
            // Correct the index with the number of already tested ones inferior
            // to it. This works because the set is ordered
            std::for_each(operandDataSourceIndexes.begin(),
                          operandDataSourceIndexes.end(),
                          [&operandDataSourceIndex](uint64_t index) {
                              if (index <= operandDataSourceIndex)
                                  operandDataSourceIndex++;
                          });
            // Add the index to the set
            operandDataSourceIndexes.insert(operandDataSourceIndex);

            // check if the selected dataSource can provide the type requested
            // by the instruction
            if (operandDataSourceIndex == 0) {
                // Data Source is the registers
                operandFound = env.getFakeRegisters().canHandle(operandType);
            }
            else {
                // Data source is a dataHandler
                operandFound = env.getDataSources()
                                   .at(operandDataSourceIndex - 1)
                                   .get()
                                   .canHandle(operandType);
            }
        }
    }
    else if (initOperandDataSource) {
        // The operand is not constrained in type
        operandFound = true;
        // Select a location
        operandDataSourceIndex = rng.getUnsignedInt64(
            0, env.getNbDataSources() - 1 - ((forceChange) ? 1 : 0));
        if (forceChange &&
            operandDataSourceIndex >= line.getOperand(operandIdx).first) {
            operandDataSourceIndex += 1;
        }
    }

    // The data source can provide the required data type
    uint64_t operandLocation = line.getOperand(operandIdx).second;
    if (operandFound && initOperandLocation) {
        // Select a location
        operandLocation = rng.getUnsignedInt64(
            0, env.getLargestAddressSpace() - 1 - ((forceChange) ? 1 : 0));
        if (forceChange &&
            operandLocation >= line.getOperand(operandIdx).second) {
            operandLocation += 1;
        }
    }

    // set line operand info
    if (operandFound) {
        line.setOperand(operandIdx, operandDataSourceIndex, operandLocation);
    }

    return operandFound;
}

void Mutator::LineMutator::initRandomCorrectLine(Program::Line& line,
                                                 Mutator::RNG& rng)
{
    const Environment& env = line.getEnvironment();

    // Select and set a destinationIndex. (can not fail)
    uint64_t destinationIndex =
        rng.getUnsignedInt64(0, env.getNbRegisters() - 1);
    line.setDestinationIndex(
        destinationIndex); // Should never throw.. but I did not deactivate the
                           // check anyway.

    // Select and set all parameter values. (can not fail)
    for (uint64_t paramIdx = 0; paramIdx < env.getMaxNbParameters();
         paramIdx++) {
        Parameter param(
            (int16_t)(rng.getUnsignedInt64(0, ((int64_t)PARAM_INT_MAX -
                                               (int64_t)PARAM_INT_MIN)) +
                      (int64_t)PARAM_INT_MIN));
        line.setParameter(paramIdx, param);
    }

    // Select an instruction.
    uint64_t instructionIndex =
        rng.getUnsignedInt64(0, (env.getNbInstructions() - 1));
    // Get the instruction
    const Instructions::Instruction& instruction =
        env.getInstructionSet().getInstruction(instructionIndex);
    // Set the instructionIndex
    line.setInstructionIndex(
        instructionIndex); // Should never throw.. but I did not deactivate the
                           // check anyway.

    // Select operands needed by the instruction
    uint64_t operandIdx = 0;
    for (; operandIdx < env.getMaxNbOperands(); operandIdx++) {

        // Check if all operands were tested (and none were valid)
        initRandomCorrectLineOperand(instruction, line, operandIdx, true, true,
                                     false, rng);

        // This operation can (no longer) fail since commit abd7cd since
        // all Instruction are vetted when building the Environment
    }
}

void Mutator::LineMutator::alterCorrectLine(Program::Line& line,
                                            Mutator::RNG& rng)
{
    // Generate a random int to select the modified part of the line
    const LineSize lineSize = line.getEnvironment().getLineSize();
    uint64_t selectedBit = rng.getUnsignedInt64(0, lineSize - 1);

    // Find the selected part
    bool checkValidity = false;
    if (selectedBit < lineSize.nbInstructionBits) {
        // InstructionIndex
        // Select a random Instruction (different from the current one)
        const uint64_t currentInstructionIndex = line.getInstructionIndex();
        uint64_t newInstructionIndex = rng.getUnsignedInt64(
            0, line.getEnvironment().getNbInstructions() - 2);
        newInstructionIndex +=
            (newInstructionIndex >= currentInstructionIndex) ? 1 : 0;
        line.setInstructionIndex(newInstructionIndex);

        // Check if operands are compatible with the new instruction.
        // If not: mutate them
        const Instructions::Instruction& instruction =
            line.getEnvironment().getInstructionSet().getInstruction(
                newInstructionIndex);
        for (uint64_t i = 0; i < instruction.getNbOperands(); i++) {
            const std::type_info& type =
                instruction.getOperandTypes().at(i).get();
            uint64_t dataSourceIndex = line.getOperand(i).first;
            bool isValid = false;
            if (dataSourceIndex == 0) {
                // regsister
                isValid =
                    (line.getEnvironment().getFakeRegisters().canHandle(type));
            }
            else {
                // not register
                const Data::DataHandler& dataSource =
                    line.getEnvironment()
                        .getDataSources()
                        .at(dataSourceIndex - 1)
                        .get();
                isValid = dataSource.canHandle(type);
            }
            // Alter the operand if needed
            if (!isValid) {
                // Force only the change of data source (location can remain
                // unchanged thanks to scaling). This can never fail since there
                // is a check for Instructions viability during the Environment
                // Construction. Hence, eithed isValid is true, OR a valid
                // dataSource will be found among other data sources.
                initRandomCorrectLineOperand(instruction, line, i, true, false,
                                             true, rng);
            }
        }
    }
    else if (selectedBit <
             lineSize.nbInstructionBits + lineSize.nbDestinationBits) {
        // DestinationIndex
        // Select a random destination (different from the current one)
        const uint64_t currentDestinationIndex = line.getDestinationIndex();
        uint64_t newDestinationIndex =
            rng.getUnsignedInt64(0, line.getEnvironment().getNbRegisters() - 2);
        newDestinationIndex +=
            (newDestinationIndex >= currentDestinationIndex) ? 1 : 0;
        line.setDestinationIndex(newDestinationIndex);
    }
    else if (selectedBit < lineSize.nbInstructionBits +
                               lineSize.nbDestinationBits +
                               lineSize.nbOperandsBits) {
        // Which operand is selected
        // Equal position of selectedBit within operand bits, divided by the
        // total number of bits per operand.
        const uint64_t operandIndex =
            (selectedBit -
             (lineSize.nbInstructionBits + lineSize.nbDestinationBits)) /
            (lineSize.nbOperandDataSourceIndexBits +
             lineSize.nbOperandLocationBits);
        const uint64_t currentOperandDataSourceIndex =
            line.getOperand(operandIndex).first;
        const uint64_t currentOperandLocation =
            line.getOperand(operandIndex).second;
        const Instructions::Instruction& instruction =
            line.getEnvironment().getInstructionSet().getInstruction(
                line.getInstructionIndex());

        // Operands dataSourceIndex or Location
        // Same as before, but with modulo instead of division.
        // Result of modulo is compared with the number of bits per operand for
        // the operandSourceIndex encoding
        if (((selectedBit -
              (lineSize.nbInstructionBits + lineSize.nbDestinationBits)) %
             (lineSize.nbOperandDataSourceIndexBits +
              lineSize.nbOperandLocationBits)) <
            lineSize.nbOperandDataSourceIndexBits) {
            // Operand data source index
            initRandomCorrectLineOperand(instruction, line, operandIndex, true,
                                         false, true, rng);
        }
        else {
            // Location (no fail thanks to scaling)
            initRandomCorrectLineOperand(instruction, line, operandIndex, false,
                                         true, true, rng);
        }
    }
    else {
        // Parameters
        // Which Parameter is selected
        const uint64_t parameterIndex =
            (selectedBit - (lineSize.totalNbBits - lineSize.nbParametersBits)) /
            (lineSize.nbParametersBits /
             line.getEnvironment().getMaxNbParameters());
        // should not cause any division by zero since if there is no parameter
        // in an environment, this code should never be reached.

        // Select a random parameterValue
        // (do not bother to make it different the probability is too low and
        // it will be detected through neutrality tests)
        Parameter newParameter =
            (int16_t)(rng.getUnsignedInt64(0, ((int64_t)PARAM_INT_MAX -
                                               (int64_t)PARAM_INT_MIN)) +
                      (int64_t)PARAM_INT_MIN);
        line.setParameter(parameterIndex, newParameter);
    }
}
