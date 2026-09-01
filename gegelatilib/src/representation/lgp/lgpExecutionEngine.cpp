/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#include "representation/lgp/lgpExecutionEngine.h"

void Representation::LGP::LGPExecutionEngine::executeCurrentLine()
{
    std::vector<Data::DataView> operands;

    // Get everything needed (may throw)
    const LGPLine& line = this->getCurrentLine();
    const Instructions::Instruction& instruction =
        this->getCurrentInstruction();
    this->fetchCurrentOperands(operands);


    //this->registers.setDataAt(typeid(double), line.getDestinationIndex(),
    //                          instruction.execute(operands).view());
}

std::vector<double> Representation::LGP::LGPExecutionEngine::execute()
{
    // Reset registers and programCounter
    this->registers.resetData();


    iterateThroughtProgram(this->ignoreException);

    const std::vector<size_t>& outputIndices = dynamic_cast<const LgpIndividual&>(this->executedIndividual->get()).getOutputIndices();
    std::vector<double> result;
    for(size_t idx = 0; idx < outputIndices.size(); idx++){
        // cast to primitiveType<double> to enable cast to double.
        result.push_back(this->registers.getDataAt(typeid(double), outputIndices[idx])
                 .getScalar<double>());
    }

    if(this->outputs.sizeContinuous() == 0){
        if(this->outputs.sizeDiscrete() > 1){
            Output::convertContinuousToDiscreteOutputs(result, this->outputs);
            return result;
        } else {
            size_t max_index = 0;
            double max_value = result[0];

            for (size_t i = 1; i < this->outputs.front().getNbValues(); ++i) {
                if (result[i] > max_value) {
                    max_value = result[i];
                    max_index = i;
                }
            }
            return {(double)max_index};
        }
    } else {
        // Returns the register values
        // TODO ACTIVATION FUNCTIONS
        if(outputIndices.front() == 20000) {

            std::string str = "Output of " + std::to_string(outputIndices.front()) + " R: " + std::to_string(result.front());
            result = Utils::ActivationFunctions::scaleOutputValues(result, this->outputs, Utils::ActivationFunction::TANH);
            str = str + "  -- A: " + std::to_string(result.front());
            std::cout<<str<<std::endl;
        } else {
            result = Utils::ActivationFunctions::scaleOutputValues(result, this->outputs, Utils::ActivationFunction::TANH);

        }
        return result;
    }
}

void Representation::LGP::LGPExecutionEngine::processLine()
{
    this->executeCurrentLine();
}
