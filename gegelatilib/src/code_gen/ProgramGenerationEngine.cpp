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


#ifdef CODE_GENERATION

#include "code_gen/ProgramGenerationEngine.h"
#include <cxxabi.h>

const std::regex CodeGen::ProgramGenerationEngine::operand_regex("(\\$[0-9]*)");
const std::string CodeGen::ProgramGenerationEngine::nameRegVariable("reg");
const std::string CodeGen::ProgramGenerationEngine::nameDataVariable("in");

void CodeGen::ProgramGenerationEngine::generateCurrentLine(){
    const Program::Line& line = this->getCurrentLine();
    const Instructions::Instruction& instruction = this->getCurrentInstruction();
    auto prtIns = dynamic_cast<const Instructions::PrintableInstruction*>(&instruction);

    if (prtIns != nullptr){
        std::string codeLine = completeFormat(*prtIns);
        fileC << "\t" << codeLine << std::endl;
    }
    else{
        throw std::runtime_error("The pointer on the instruction cannot be"
                                 " converted to a pointer on a printable"
                                 "instruction.");
    }

}
void CodeGen::ProgramGenerationEngine::generateProgram(uint64_t progID, const bool ignoreException)
{
    fileC << "\ndouble P" << progID <<"(){" << std::endl;
    fileH << "double P" << progID <<"();" << std::endl;

    // instantiate register
    fileC << "\tdouble "<< nameRegVariable <<"[" << program->getEnvironment().getNbRegisters() << "] = {0";
    for (int i = 1; i < program->getEnvironment().getNbRegisters(); ++i) {
        fileC << ", 0";
    }
    fileC << "};" << std::endl;
    this->programCounter = 0;
//todo embarqué dans une fonction globale avec une operateur() ? avec un objet foncteur
    // Iterate over the lines of the Program
    bool hasNext = this->program->getNbLines() > 0;

    // Skip first lines if they are introns.
    if (hasNext && this->program->isIntron(0)) {
        hasNext = this->next();
    }

    // Execute useful lines
    while (hasNext) {
        try {
            // generate the current line
            this->generateCurrentLine(); //todo utilisez un objet foncteur à la place
        }
        catch (std::out_of_range e) {
            if (!ignoreException) {
                throw; // rethrow
            }
        }

        // Increment the programCounter.
        hasNext = this->next();
    };
#ifdef DEBUG
    fileC << "#ifdef DEBUG" << std::endl;
    fileC << "\tprintf(\"P" << progID << " : reg[0] = %lf \\n\", reg[0]);" << std::endl;
    fileC << "#endif" << std::endl;
#endif
    fileC << "\treturn reg[0];\n}" << std::endl;

}
std::string CodeGen::ProgramGenerationEngine::completeFormat(
    const Instructions::PrintableInstruction& instruction) const
{
    const std::string& format = instruction.getFormat();
    const Program::Line& line = this->getCurrentLine(); // throw std::out_of_range
    std::string codeLine(format);
    std::string operandValue;
    for(auto itr = std::sregex_iterator(format.begin(), format.end(), operand_regex); itr != std::sregex_iterator(); ++itr){
        const std::string& match = (*itr).str();
        auto pos = codeLine.find(match);
        // get number after character '$'
        int idx = std::stoi(match.substr(1));
        if(idx > 0) {
            const uint64_t operandLocation = this->getOperandLocation(idx-1);

            // if number > 0 means that it's in the left side of the operation
            if(line.getOperand(idx-1).first == 0){
                //operand value is an intern register
                 operandValue = nameRegVariable + "[" +
                     std::to_string(operandLocation) + "]";

            }
            else {
                //operandValue come from the environment
                 operandValue = nameDataVariable + std::to_string(line.getOperand(idx-1).first) + "[" +
                     std::to_string(operandLocation) + "]";
            }
        }
        else{
            // if number == 0 it corresponds to the result of the function
            operandValue = nameRegVariable + "[" + std::to_string(line.getDestinationIndex()) + "]";
        }
        codeLine.replace(pos, match.size(), operandValue);
    }
    return codeLine;
}
void CodeGen::ProgramGenerationEngine::initGlobalVar(){

    for (int i = 1; i < this->dataScsConstsAndRegs.size(); ++i) {

        const Data::DataHandler& d = this->dataScsConstsAndRegs.at(i);

        std::string type = d.getTemplateType();
        fileC << "extern " << type << "* in" << i << ";" << std::endl;
    }
}

#endif // CODE_GENERATION