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

#include <algorithm>
#include <new>
#include <set>
#include <stdexcept>
#include <typeinfo>

#include "data/primitiveTypeArray.h"

#include "program/program.h"

Program::Program::~Program()
{
    while (!lines.empty()) {
        Line* line = lines.back().first;
        delete line;
        lines.pop_back();
    }
}

Program::Line& Program::Program::addNewLine()
{
    return this->addNewLine(this->getNbLines());
}

Program::Line& Program::Program::addNewLine(const uint64_t idx)
{
    if (idx > this->getNbLines()) {
        throw std::out_of_range(
            "Attempting to insert a line beyond the program end.");
    }
    // Allocate the zero-filled memory
    Line* newLine = new Line(this->environment);
    // new line is not marked as an intron by default
    this->lines.insert(lines.begin() + idx, {newLine, false});

    return *newLine;
}

void Program::Program::removeLine(const uint64_t idx)
{
    delete this->lines.at(idx).first; // throws std::out_of_range on bad index.
    this->lines.erase(this->lines.begin() + idx);
}

void Program::Program::swapLines(const uint64_t idx0, const uint64_t idx1)
{
    if (idx0 >= this->getNbLines() || idx1 >= this->getNbLines()) {
        throw std::out_of_range(
            "Attempting to swap a line beyond the program end.");
    }

    std::iter_swap(this->lines.begin() + idx0, this->lines.begin() + idx1);
}

const Environment& Program::Program::getEnvironment() const
{
    return this->environment;
}

size_t Program::Program::getNbLines() const
{
    return this->lines.size();
}

const Program::Line& Program::Program::getLine(uint64_t index) const
{
    return *this->lines.at(index)
                .first; // throws std::out_of_range on bad index.
}

Program::Line& Program::Program::getLine(uint64_t index)
{
    return *this->lines.at(index)
                .first; // throws std::out_of_range on bad index.
}

bool Program::Program::isIntron(uint64_t index) const
{
    return this->lines.at(index)
        .second; // throws std::out_of_range on bad index.
}

uint64_t Program::Program::identifyIntrons()
{
    // Create fake registers to identify accessed addresses.
    const Data::DataHandler& fakeRegisters =
        this->environment.getFakeDataSources().at(0);
    // Number of introns within the Program.
    uint64_t nbIntrons = 0;
    // Set of useful register
    std::set<uint64_t> usefulRegisters;
    // Start with only register 0
    usefulRegisters.insert(0);

    // Scan program lines backward
    auto backIter = this->lines.rbegin();
    while (backIter != this->lines.rend()) {
        // Check if the currentLine output is within usefulRegisters
        Line* currentLine = backIter->first;
        uint64_t destinationIndex = currentLine->getDestinationIndex();
        auto destinationRegister = usefulRegisters.find(destinationIndex);
        if (destinationRegister != usefulRegisters.end()) {
            // The Line is useful (i.e. not an introns)
            backIter->second = false;

            // Remove the destination register from the list of useful operands
            usefulRegisters.erase(*destinationRegister);

            // Add register operands to the list of useful registers
            const Instructions::Instruction& instruction =
                this->environment.getInstructionSet().getInstruction(
                    currentLine->getInstructionIndex());
            size_t nbOperands = instruction.getNbOperands();
            for (auto idxOperand = 0; idxOperand < nbOperands; idxOperand++) {
                // Is the operand a register (i.e. its index is 0)
                if (currentLine->getOperand(idxOperand).first == 0) {
                    // The operand is a register, add the accessed register to
                    // the list of useful registers.
                    const std::type_info& operandType =
                        instruction.getOperandTypes().at(idxOperand);
                    uint64_t location =
                        currentLine->getOperand(idxOperand).second;
                    uint64_t registerIdx =
                        location % fakeRegisters.getAddressSpace(operandType);
                    std::vector<size_t> accessedAddresses =
                        fakeRegisters.getAddressesAccessed(operandType,
                                                           registerIdx);
                    for (size_t accessedAddress : accessedAddresses) {
                        usefulRegisters.insert(accessedAddress);
                    }
                }
            }
        }
        else {
            // The destination of the line is not within useful registers
            // the line does not contribute to the result of the Program
            // it is an intron.
            backIter->second = true;
            nbIntrons++;
        }

        backIter++;
    }

    return nbIntrons;
}

const Data::ConstantHandler& Program::Program::cGetConstantHandler() const
{
    return this->constants;
}

Data::ConstantHandler& Program::Program::getConstantHandler()
{
    return this->constants;
}

const Data::Constant Program::Program::getConstantAt(size_t index) const
{
    std::shared_ptr<const Data::Constant> value =
        this->constants.getDataAt(typeid(Data::Constant), index)
            .getSharedPointer<const Data::Constant>();
    return *value;
}

bool Program::Program::hasIdenticalBehavior(const Program& other) const
{
    size_t thisLineIdx = 0;
    size_t otherLineIdx = 0;

    auto nextNonIntronIdx = [](const Program& p, size_t& lineIdx) {
        while (lineIdx < p.getNbLines() && p.isIntron(lineIdx)) {
            lineIdx++;
        }
    };

    // Look for the first non intron line in both programs
    nextNonIntronIdx(*this, thisLineIdx);
    nextNonIntronIdx(other, otherLineIdx);

    // Scan the two programs
    while (thisLineIdx < this->getNbLines() &&
           otherLineIdx < other.getNbLines()) {

        // Check that two non-intron lines were reached
        if (thisLineIdx < this->getNbLines() &&
            otherLineIdx < other.getNbLines()) {

            // Compare the two lines
            const Line& thisLine = this->getLine(thisLineIdx);
            const Line& otherLine = other.getLine(otherLineIdx);

            if (thisLine != otherLine) {
                return false;
            }

            // If lines are referencing Constant, compare the values
            // of these Constants
            if (this->environment.getNbConstant() > 0) {
                // Get Instruction
                const Instructions::Instruction& instruction =
                    this->environment.getInstructionSet().getInstruction(
                        thisLine.getInstructionIndex());

                // Check operands
                for (auto operandIdx = 0;
                     operandIdx < instruction.getNbOperands(); operandIdx++) {
                    // Is the operand from the Constant data source.
                    if (thisLine.getOperand(operandIdx).first == 1) {
                        // Check equality of constants
                        Data::Constant thisCste = this->getConstantAt(
                            thisLine.getOperand(operandIdx).second);
                        Data::Constant otherCste = other.getConstantAt(
                            thisLine.getOperand(operandIdx).second);
                        if (thisCste != otherCste) {
                            return false;
                        }
                    }
                }
            }

            // Look for the next non intron line in both programs
            thisLineIdx++;
            nextNonIntronIdx(*this, thisLineIdx);
            otherLineIdx++;
            nextNonIntronIdx(other, otherLineIdx);
        }
    }

    if ((thisLineIdx < this->getNbLines()) ^
        (otherLineIdx < other.getNbLines())) {
        // XOR: only one of the two program reached its last line
        return false;
    }

    // Everything was identical, return true
    return true;
}