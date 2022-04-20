/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Mickael Dardaillon <mdardail@insa-rennes.fr> (2022)
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

#include "codeGen/tpgGenerationEngineFactory.h"
#include "codeGen/tpgStackGenerationEngine.h"
#include "codeGen/tpgSwitchGenerationEngine.h"

CodeGen::TPGGenerationEngineFactory::TPGGenerationEngineFactory()
    : TPGGenerationEngineFactory(switchMode){};

CodeGen::TPGGenerationEngineFactory::TPGGenerationEngineFactory(
    enum generationEngineMode mode)
{
    this->mode = mode;
}

std::unique_ptr<CodeGen::TPGGenerationEngine> CodeGen::
    TPGGenerationEngineFactory::create(const std::string& filename,
                                       const TPG::TPGGraph& tpg,
                                       const std::string& path)
{
    if (this->mode == stackMode) {
        int stack = tpg.getEdges().size(); // use upper bound on stack size
        return std::make_unique<TPGStackGenerationEngine>(filename, tpg, path,
                                                          stack);
    }
    else if (this->mode == switchMode) {
        return std::make_unique<TPGSwitchGenerationEngine>(filename, tpg, path);
    }
    else {
        return nullptr;
    }
}

#endif // CODE_GENERATION