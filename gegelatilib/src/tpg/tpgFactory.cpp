/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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

#include "tpg/tpgFactory.h"
#include "tpg/tpgExecutionEngine.h"
#include "tpg/tpgGraph.h"

std::shared_ptr<TPG::TPGGraph> TPG::TPGFactory::createTPGGraph(
    const Environment& env) const
{
    return std::make_shared<TPG::TPGGraph>(env, std::make_unique<TPGFactory>());
}

TPG::TPGTeam* TPG::TPGFactory::createTPGTeam() const
{
    return new TPG::TPGTeam();
}

TPG::TPGAction* TPG::TPGFactory::createTPGAction(const uint64_t actID,
                                                 const uint64_t actClass) const
{
    return new TPG::TPGAction(actID, actClass);
}

std::unique_ptr<TPG::TPGEdge> TPG::TPGFactory::createTPGEdge(
    const TPGVertex* src, const TPGVertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    return std::make_unique<TPG::TPGEdge>(src, dest, prog);
}

std::unique_ptr<TPG::TPGExecutionEngine> TPG::TPGFactory::
    createTPGExecutionEngine(const Environment& env, Archive* arch) const
{
    return std::make_unique<TPG::TPGExecutionEngine>(env, arch);
}
