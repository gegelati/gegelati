/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#include "representation/tgp/tgpLineMutator.h"


bool Representation::TGP::TGPLineMutator::initRandomCorrectLineOperand(
    const Instructions::Instruction& instruction, LGP::LGPLine& line, bool maxDepthReached,
    const uint64_t& operandIdx, const bool initOperandDataSource,
    const bool initOperandLocation, const bool forceChange, RNG::RNG& rng)
{
    const LGP::LGPEnvironment& env = line.getEnvironment();
    uint64_t operandDataSourceIndex = line.getOperand(operandIdx).first;
    bool operandFound = !initOperandDataSource;


    // Is the operand constrained in type?
    if (initOperandDataSource && operandIdx < instruction.getNbOperands()) {
        // Select an operand
        // The type of operand needed
        const std::type_info& operandType =typeid(double);
                //instruction.getOperandTypes().at(operandIdx).get();

        // keep a record of tested indexes
        std::set<uint64_t> operandDataSourceIndexes;
        if (forceChange) {
            operandDataSourceIndexes.insert(operandDataSourceIndex);
        }

        size_t nbDataSources = env.getNbDataSources();
        // Registers unavailable
        if(maxDepthReached) {
            operandDataSourceIndexes.insert(0);
            nbDataSources - 1;
        } 

        while (!operandFound &&
               operandDataSourceIndexes.size() < nbDataSources) {

            // Select an operandDataSourceIndex
            operandDataSourceIndex = rng.getUnsignedInt64(0, nbDataSources - 1 - operandDataSourceIndexes.size());
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
            operandFound = env.getFakeDataSources()
                               .at(operandDataSourceIndex)
                               .get()
                               .canHandle(operandType);

        }
    }
    else if (initOperandDataSource) {
        // The operand is not constrained in type
        operandFound = true;
        
        // Select a location
        operandDataSourceIndex = rng.getUnsignedInt64(((!maxDepthReached) ? 0 : 1), env.getNbDataSources() - 1);
    }

    // The data source can provide the required data type
    uint64_t operandLocation = line.getOperand(operandIdx).second;
        
    if (operandFound && initOperandLocation) {
        if(operandDataSourceIndex != 0) {
            // Select a location
            operandLocation = rng.getUnsignedInt64(
                0, env.getLargestAddressSpace() - 1 - ((forceChange) ? 1 : 0));

            if (forceChange &&
                operandLocation >= line.getOperand(operandIdx).second) {
                operandLocation += 1;
            }
        // Register case
        } else {
            // Select a location
            operandLocation = env.getMaxNbOperands() * line.getDestinationIndex() + 1 + operandIdx;
        }
    } else if (operandFound && operandDataSourceIndex == 0) {
            // Select a location
            operandLocation = env.getMaxNbOperands() * line.getDestinationIndex() + 1 + operandIdx;
    }

    // set line operand info
    if (operandFound) {
        line.setOperand(operandIdx, operandDataSourceIndex, operandLocation);
    }

    return operandFound;
}


bool Representation::TGP::TGPLineMutator::isLineCorrect(LGP::LGPLine& line, bool maxDepthReached)
{
    if(line.getEnvironment().getMaxNbOperands() > 2) {
        return false;
    }
    for(size_t idx = 0; idx < line.getEnvironment().getMaxNbOperands(); idx ++) {
        if(maxDepthReached && line.getOperand(idx).first == 0) {
            return false;
        }
        if(line.getOperand(idx).first == 0 && line.getOperand(idx).second != 2 * line.getDestinationIndex() + 1 + idx) {
            return false;
        } 
    }
    return true;
}

void Representation::TGP::TGPLineMutator::initRandomCorrectLine(LGP::LGPLine& line, size_t idxRegister, bool maxDepthReached, RNG::RNG& rng)
{
    const LGP::LGPEnvironment& env = line.getEnvironment();

    // Select and set a destinationIndex. (can not fail)
    uint64_t destinationIndex = idxRegister;
    line.setDestinationIndex(
        destinationIndex); // Should never throw.. but I did not deactivate the
                           // check anyway.

    bool success = false;
    while(!success) {
        success = true;
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
            if(!initRandomCorrectLineOperand(instruction, line, maxDepthReached, operandIdx, true, true,
                                        false, rng)){
                success = false;
            }

            // This operation can (no longer) fail since commit abd7cd since
            // all Instruction are vetted when building the Environment

            // THis can fail again if registers are not available
        }
    }


    if(!this->isLineCorrect(line, maxDepthReached)) {
        throw std::runtime_error("TGPLineMutator::initRandomCorrectLine: line is not correct for TreeBased GP after intialisation");
    }
}

void Representation::TGP::TGPLineMutator::alterCorrectLine(LGP::LGPLine& line, bool maxDepthReached, RNG::RNG& rng)
{
    // Generate a random int to select the modified part of the line
    const LGP::LGPLineSize lineSize = line.getEnvironment().getLineSize();

    // Ignore size destination that cannot be mutated for TGP
    uint64_t selectedBit = rng.getUnsignedInt64(0, lineSize - lineSize.nbDestinationBits - 1);

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
            const std::type_info& type =typeid(double);
                //instruction.getOperandTypes().at(i).get();
            uint64_t dataSourceIndex = line.getOperand(i).first;
            bool isValid = false;
            const Data::DataHandler& dataSource = line.getEnvironment()
                                                      .getFakeDataSources()
                                                      .at(dataSourceIndex)
                                                      .get();
            isValid = dataSource.canHandle(type);
            // Alter the operand if needed
            if (!isValid) {
                // Force only the change of data source (location can remain
                // unchanged thanks to scaling). This can never fail since there
                // is a check for Instructions viability during the Environment
                // Construction. Hence, eithed isValid is true, OR a valid
                // dataSource will be found among other data sources.
                initRandomCorrectLineOperand(instruction, line, maxDepthReached, i, true, false,
                                             true, rng);
            }
        }
    }
    else {
        selectedBit += lineSize.nbDestinationBits;
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
            initRandomCorrectLineOperand(instruction, line, maxDepthReached, operandIndex, true,
                                         false, true, rng);
        }
        else {
            // Location (no fail thanks to scaling)
            initRandomCorrectLineOperand(instruction, line, maxDepthReached, operandIndex, false,
                                         true, true, rng);
        }
    }
    if(!this->isLineCorrect(line, maxDepthReached)) {
        throw std::runtime_error("TGPLineMutator::alterCorrectLine: line is not correct for TreeBased GP after mutation");
    }
}
