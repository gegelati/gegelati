/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include "representation/lgp/lgpPolicyStats.h"
#include <algorithm>
#include <numeric>

void Representation::LGP::LGPPolicyStats::analyzeLine(const LGPLine& line)
{
    auto instructionIdx = line.getInstructionIndex();

    this->nbUsagePerInstruction[instructionIdx]++;

    const Instructions::Instruction& instruction =
        this->environment.getInstructionSet().getInstruction(instructionIdx);

    for (size_t operandIdx = 0; operandIdx < instruction.getNbOperands();
         operandIdx++) {
        const std::pair<size_t, size_t>& rawOperand =
            line.getOperand(operandIdx);
        const std::type_info& operandType =
            instruction.getOperandTypes().at(operandIdx).get();
        const Data::DataHandler& dHandler =
            dataSourcesAndRegisters.at(rawOperand.first).get();
        size_t scaledLocation =
            dHandler.scaleLocation(rawOperand.second, operandType);
        std::vector<size_t> accessedLocations =
            dHandler.getAddressesAccessed(operandType, scaledLocation);

        for (size_t accessedLocation : accessedLocations) {
            this->nbUsagePerDataLocation[{rawOperand.first, accessedLocation}]++;
        }
    }

}

void Representation::LGP::LGPPolicyStats::analyzePolicy(const Individual& agent)
{

    // Get tpgIndividual if agent is one, else throw
    const LgpIndividual& lgpIndividual = dynamic_cast<const LgpIndividual&>(agent);
    if (&lgpIndividual == nullptr) {
        throw std::invalid_argument("PolicyStats can only analyze LgpIndividual");
    }

    // Check if the Program was already analyzed
    auto programIterator = this->nbUsePerProgram.find(lgpIndividual);
    if (programIterator != this->nbUsePerProgram.end()) {
        programIterator->second++;
        return;
    }

    this->nbUsePerProgram.emplace(lgpIndividual, 1);
    this->nbLinesPerProgram.push_back(lgpIndividual.getNbLines());

    size_t nbIntronLines = 0;
    for (auto lineIdx = 0; lineIdx < lgpIndividual.getNbLines(); lineIdx++) {
        if (!lgpIndividual.isIntron(lineIdx)) {
            this->analyzeLine(lgpIndividual.getLine(lineIdx));
        }
        else {
            nbIntronLines++;
        }
    }
    this->nbIntronPerProgram.push_back(nbIntronLines);
}

std::string Representation::LGP::LGPPolicyStats::specificInfos() const 
{
    std::ostringstream os;
    
    auto sumVec = [](const std::vector<size_t>& vec) {
        return std::accumulate(vec.cbegin(), vec.cend(), (size_t)0);
    };
    auto averageVec = [&sumVec](const std::vector<size_t>& vec) {
        return vec.empty() ? 0.0 : (double)sumVec(vec) / (double)vec.size();
    };

    os << "Programs:\t" << this->nbUsePerProgram.size() << std::endl;
    os << "Line/prog:\t" << averageVec(this->nbLinesPerProgram)
        << std::endl;
    os << "Intr/prog:\t" << averageVec(this->nbIntronPerProgram)
        << std::endl;
    os << "Use/prog:\t";
    if (!this->nbUsePerProgram.empty()) {
        os << (double)std::accumulate(this->nbUsePerProgram.cbegin(),
                                        this->nbUsePerProgram.cend(),
                                        size_t(0),
                                        [](size_t accu, const auto& val) {
                                            return accu + val.second;
                                        }) /
                    (double)this->nbUsePerProgram.size();
    }
    os << std::endl;

    os << "Use/instr:\t";
    if (!this->nbUsagePerInstruction.empty()) {
        os << (double)std::accumulate(
                    this->nbUsagePerInstruction.cbegin(),
                    this->nbUsagePerInstruction.cend(), size_t(0),
                    [](size_t accu, const std::pair<size_t, size_t>& val) {
                        return accu + val.second;
                    }) /
                    (double)this->nbUsagePerInstruction.size();
    }
    os << ": ";
    for (const auto& val : this->nbUsagePerInstruction) {
        os << "{" << val.first << "," << val.second << "}";
    }
    os << std::endl << std::endl;

    os << "## Data info";
    if (!this->nbUsagePerDataLocation.empty()) {
        size_t currentDHandler =
            this->nbUsagePerDataLocation.begin()->first.first - 1;
        auto& currentNbUsagePerDataLocation = this->nbUsagePerDataLocation;
        std::for_each(
            this->nbUsagePerDataLocation.cbegin(),
            this->nbUsagePerDataLocation.cend(),
            [&os, &currentDHandler, &currentNbUsagePerDataLocation](const auto& entry) {
                if (entry.first.first != currentDHandler) {
                    os << "\n\n### DataHandler " << entry.first.first
                        << std::endl;
                    currentDHandler = entry.first.first;
                    size_t nbLocation = 0;
                    auto nbAccess = std::accumulate(
                        currentNbUsagePerDataLocation.cbegin(),
                        currentNbUsagePerDataLocation.cend(), size_t(0),
                        [&currentDHandler, &nbLocation](size_t accu,
                                                        const auto& val) {
                            if (val.first.first == currentDHandler) {
                                nbLocation++;
                                return accu + val.second;
                            }
                            else {
                                return accu;
                            }
                        });
                    os << "Accesses:\t" << nbAccess << std::endl;
                    os << "Locations:\t" << nbLocation << std::endl;
                }
                os << "{" << entry.first.second << "," << entry.second
                    << "} ";
            });
    }
    os << std::endl;
    
    return os.str();
}