/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

#include <stdexcept>

#include "program/line.h"

const Environment& Program::Line::getEnvironment() const
{
    return this->environment;
}

uint64_t Program::Line::getDestinationIndex() const
{
    return this->destinationIndex;
}

bool Program::Line::setDestinationIndex(uint64_t dest, bool check)
{
    if (check && dest >= this->environment.getNbRegisters()) {
        return false;
    }
    this->destinationIndex = dest;
    return true;
}

uint64_t Program::Line::getInstructionIndex() const
{
    return this->instructionIndex;
}

bool Program::Line::setInstructionIndex(uint64_t instr, bool check)
{
    if (check && instr >= this->environment.getNbInstructions()) {
        return false;
    }
    this->instructionIndex = instr;
    return true;
}

const std::pair<uint64_t, uint64_t>& Program::Line::getOperand(
    const uint64_t idx) const
{
    if (idx >= this->environment.getMaxNbOperands()) {
        throw std::range_error("Attempting to access an non-existing operand.");
    }

    return this->operands[idx];
}

bool Program::Line::setOperand(const uint64_t idx, const uint64_t dataIndex,
                               const uint64_t location, const bool check)
{
    if (idx >= this->environment.getMaxNbOperands()) {
        throw std::range_error("Attempting to set an non-existing operand.");
    }

    if (check) {
        // Check data Index
        if (dataIndex >= this->environment.getNbDataSources()) {
            return false;
        }
        // Check location
        if (location >= this->environment.getLargestAddressSpace()) {
            return false;
        }
    }

    this->operands[idx].first = dataIndex;
    this->operands[idx].second = location;

    return true;
}
