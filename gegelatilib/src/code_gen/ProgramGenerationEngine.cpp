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

#define CODE_GENERATION
#ifdef CODE_GENERATION

#include "code_gen/ProgramGenerationEngine.h"

const std::regex Program::ProgramGenerationEngine::operand_regex("(\\$[0-9]*)");
const std::string Program::ProgramGenerationEngine::nameRegVariable("reg");

void Program::ProgramGenerationEngine::generateCurrentLine()
{
    std::vector<Data::UntypedSharedPtr> operand;

    const Line& line = this->getCurrentLine();
    const Instructions::Instruction& instruction = this->getCurrentInstruction();
    this->fetchCurrentOperands(operand);

    if(instruction.isPrintable()){
        auto prtIns = dynamic_cast<const Instructions::PrintableInstruction*>(&instruction);
        if (prtIns != nullptr){
            std::string codeLine = completeFormat(*prtIns,operand);
            file << "\t" << codeLine << std::endl;
        }
        else{
            throw std::runtime_error("The pointer on the instruction cannot be"
                                     " converted to a pointer on a printable"
                                     "instruction.");
        }
    }
    else{
        throw std::runtime_error("Could not generate the line, the line is not printable");
    }

}
void Program::ProgramGenerationEngine::generateProgram(const bool ignoreException)
{
    // print function (signature) double P...(data::){
    file << "\ndouble P1(){" << std::endl;

    // instanciate register
    file << "\tdouble "<< nameRegVariable <<"[" << program->getEnvironment().getNbRegisters() << "];" << std::endl;
    this->programCounter = 0;

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
            this->generateCurrentLine();
        }
        catch (std::out_of_range e) {
            if (!ignoreException) {
                throw; // rethrow
            }
        }

        // Increment the programCounter.
        hasNext = this->next();
    };
    file << "\treturn reg[0];\n}" << std::endl;


}
std::string Program::ProgramGenerationEngine::completeFormat(
    const Instructions::PrintableInstruction& ins, std::vector<Data::UntypedSharedPtr> operand) const
{
    const std::string& format = ins.getFormat();
    std::string codeLine(format);
    std::string reg;
    for(auto itr = std::sregex_iterator(format.begin(), format.end(), operand_regex); itr != std::sregex_iterator(); ++itr){
        const std::string& match = (*itr).str();
        auto pos = codeLine.find(match);
        // get number after character '$'
        int idx = std::stoi(match.substr(1));
        if(idx > 0) {
            // if number > 0 means that it's an operand
            auto ptr = operand.at(idx-1).getSharedPointer<double>();
            //todo manage pointer to the environnement instead of the value
            // pointed and register if the operand is one

            codeLine.replace(pos, match.size(), std::to_string(*ptr));
        }
        else{
            // if number == 0 it correpond to the result of the function
            std::string regNb = std::to_string(this->getCurrentLine().getDestinationIndex());
            reg = nameRegVariable + "[" + regNb + "]";
            codeLine.replace(pos, match.size(), reg);
        }
    }
    return codeLine;
}
#endif // CODE_GENERATION