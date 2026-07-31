/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

//#ifdef CODE_GENERATION
#include "representation/lgp/lgpCodeGenerationEngine.h"
#include "util/timestamp.h"

const std::regex Representation::LGP::LGPCodeGenerationEngine::operand_regex("(\\$[0-9]*)");
const std::regex Representation::LGP::LGPCodeGenerationEngine::constant_regex(
    "(\\%[0-9]*)");
const std::string Representation::LGP::LGPCodeGenerationEngine::nameRegVariable("reg");
const std::string Representation::LGP::LGPCodeGenerationEngine::nameConstantVariable("cst");
const std::string Representation::LGP::LGPCodeGenerationEngine::nameDataVariable("in");
const std::string Representation::LGP::LGPCodeGenerationEngine::nameOperandVariable("op");

void Representation::LGP::LGPCodeGenerationEngine::generateCurrentLine()
{
    const Instructions::Instruction& instruction =
        this->getCurrentInstruction();

    if (instruction.isPrintable()) {
        fileMain << "\t{" << std::endl;
        initOperandCurrentLine();
        std::string codeLine = completeFormat(instruction);
        // init
        fileMain << "\t\t" << codeLine << "\n"
              << "\t}" << std::endl;
    }
    else {
        throw std::runtime_error("The instruction is not printable, stop the "
                                 "generation of the program.");
    }
}

void Representation::LGP::LGPCodeGenerationEngine::generateProgram(
    const bool ignoreException)
{

    fileMain << "\nvoid " << this->representationName << this->representationID << "_" << this->executedAgent->get().getAgentID() << "(double* outputs){" << std::endl;

    // instantiate register
    fileMain << "\tdouble " << nameRegVariable << "["
          << env.getNbRegisters() << "] = {";
    for (int i = 0; i < env.getNbRegisters();
         ++i) {
        fileMain << "0";
        if (i < env.getNbRegisters() - 1) {
            fileMain << ", ";
        }
    }
    fileMain << "};" << std::endl;
    if (env.getNbConstants() > 0) {
        size_t nbCst = env.getNbConstants();
        fileMain << "\tdouble " << nameConstantVariable << "[" << nbCst
              << "] = {";
        for (int i = 0; i < nbCst; ++i) {
            fileMain << lgpExecutedAgent->get().getConstantAt(i).value;
            if (i < nbCst - 1) {
                fileMain << ", ";
            }
        }
        fileMain << "};" << std::endl;
    }

    iterateThroughtProgram(ignoreException);
#ifdef DEBUG
    fileMain << "#ifdef DEBUG" << std::endl;
    fileMain << "\tprintf(\"P" << progID << " : reg[0] = %lf \\n\", reg[0]);"
          << std::endl;
    fileMain << "#endif" << std::endl;
#endif

    
    const std::vector<size_t>& outputIndices = dynamic_cast<const LgpIndividual&>(this->executedAgent->get()).getOutputIndices();
    for(size_t idx = 0; idx < outputIndices.size(); idx ++ ){
        fileMain << "\toutputs["<<idx<<"] = reg["<<outputIndices[idx]<<"];\n";
    }
    // TODO ARGMAX TO ADD FOR DISCRETE OUTPUTS

    fileMain 
        << "}"<<std::endl;
}

std::string Representation::LGP::LGPCodeGenerationEngine::completeFormat(
    const Instructions::Instruction& instruction) const
{
    const std::string& printTemplate = instruction.getPrintTemplate();
    const Representation::LGP::LGPLine& line =
        this->getCurrentLine(); // throw std::out_of_range
    std::string codeLine(printTemplate);
    std::string operandValue;
    for (auto itr = std::sregex_iterator(printTemplate.begin(),
                                         printTemplate.end(), operand_regex);
         itr != std::sregex_iterator(); ++itr) {
        const std::string& match = (*itr).str();
        auto pos = codeLine.find(match);
        // get number after character '$'
        int idx = std::stoi(match.substr(1));
        if (idx > 0) {
            const uint64_t operandLocation = this->getOperandLocation(idx - 1);
            std::string operandIdx(std::to_string(idx - 1));
            operandValue = nameOperandVariable + operandIdx;
        }
        else {
            // if number == 0 it corresponds to the result of the function
            operandValue = nameRegVariable + "[" +
                           std::to_string(line.getDestinationIndex()) + "]";
        }
        codeLine.replace(pos, match.size(), operandValue);
    }
    return codeLine;
}

void Representation::LGP::LGPCodeGenerationEngine::initGlobalVar()
{
    int i;
    if (env.getNbConstants() > 0) {
        i = 2;
    }
    else {
        i = 1;
    }

    for (int cpt = 1; i < this->dataScsConstsAndRegs.size(); ++i, ++cpt) {

        const Data::DataHandler& d = this->dataScsConstsAndRegs.at(i);
        std::string type = dataPrinter.getDemangleTemplateType(d);

        fileMain << "extern " << type << "* in" << cpt << ";" << std::endl;
    }
}

void Representation::LGP::LGPCodeGenerationEngine::initOperandCurrentLine()
{
    uint64_t opIdx;
    const Representation::LGP::LGPLine& line = getCurrentLine();
    const Instructions::Instruction& instruction = getCurrentInstruction();
    for (unsigned int i = 0; i < instruction.getNbOperands(); ++i) {
        uint64_t sourceIdx = line.getOperand(i).first;
        const std::type_info& operandType =
            instruction.getOperandTypes().at(i).get();

        opIdx = this->getOperandLocation(i);
        const Data::DataHandler& dataSource = this->dataScsConstsAndRegs.at(
            sourceIdx); // Throws std::out_of_range

        fileMain << "\t\t" << instruction.getPrintablePrimitiveOperandType(i)
              << " " << nameOperandVariable << i
              << dataPrinter.printDataAt(dataSource, operandType, opIdx,
                                         getNameSourceData(sourceIdx))
              << std::endl;
    }
}

std::string Representation::LGP::LGPCodeGenerationEngine::getNameSourceData(
    const uint64_t& idx)
{
    std::string nameDataSource;
    if (idx == 0) {
        nameDataSource = nameRegVariable;
    }
    else if (this->env.getNbConstants() >
                 0 &&
             idx == 1) {
        nameDataSource = nameConstantVariable;
    }
    else {
        uint64_t varNumber = idx;
        if (this->env.getNbConstants() > 0) {
            varNumber--;
        }
        nameDataSource = nameDataVariable + std::to_string(varNumber);
    }
    return nameDataSource;
}

void Representation::LGP::LGPCodeGenerationEngine::processLine()
{
    this->generateCurrentLine();
}

//#endif // CODE_GENERATION
