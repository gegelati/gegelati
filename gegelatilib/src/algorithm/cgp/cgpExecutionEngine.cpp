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

#include "algorithm/cgp/cgpExecutionEngine.h"

std::vector<double> Algorithm::CGP::CGPExecutionEngine::execute()
{
    // Reset registers and programCounter
    this->registers.resetData();

    iterateThroughtProgram(this->ignoreException);

    // Get the output of the corresponding output index
    const std::vector<size_t>& outputIndices = dynamic_cast<const CGPAgent&>(this->executedAgent->get()).getOutputIndices();
    std::vector<double> result;
    for(size_t idx = 0; idx < this->outputs.size(); idx++){
        // cast to primitiveType<double> to enable cast to double.
        result.push_back(*(this->registers.getDataAt(typeid(double), outputIndices[idx])
                 .getSharedPointer<const double>()));
    }

    if(this->outputs.sizeContinuous() == 0){
        Output::convertContinuousToDiscreteOutputs(result, this->outputs);
        return result;
    } else {
        // Returns the register values
        // TODO ACTIVATION FUNCTIONS
        return Utils::ActivationFunctions::scaleOutputValues(result, this->outputs, Utils::ActivationFunction::TANH);
    }
}
