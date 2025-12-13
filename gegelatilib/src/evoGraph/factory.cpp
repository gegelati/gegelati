/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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

#include "evoGraph/factory.h"
#include "evoGraph/oldExecutionEngine.h"
#include "evoGraph/graph.h"

std::shared_ptr<EvoGraph::Graph> EvoGraph::GraphFactory::createGraph(
    const Environment& env) const
{
    return std::make_shared<EvoGraph::Graph>(env, std::make_unique<GraphFactory>());
}

std::shared_ptr<EvoGraph::Team> EvoGraph::GraphFactory::createTeam() const
{
    return std::make_shared<EvoGraph::Team>();
}

std::shared_ptr<EvoGraph::Action> EvoGraph::GraphFactory::createAction(
    const uint64_t id) const
{
    return std::make_shared<EvoGraph::Action>(id);
}

std::shared_ptr<EvoGraph::Edge> EvoGraph::GraphFactory::createEdge(
    std::shared_ptr<const Vertex> src, std::shared_ptr<const Vertex> dest,
    std::shared_ptr<const Algorithm::Agent> actionProgram) const
{
    return std::make_shared<EvoGraph::Edge>(src, dest, actionProgram);
}

std::shared_ptr<EvoGraph::Edge> EvoGraph::GraphFactory::createActionEdge(
    std::shared_ptr<const Vertex> src, std::shared_ptr<const Algorithm::Agent> actionProgram,
    uint64_t actionClass) const
{
    return std::make_shared<EvoGraph::ActionEdge>(src, actionProgram, actionClass);
}

std::unique_ptr<EvoGraph::OldExecutionEngine> EvoGraph::GraphFactory::
    createExecutionEngine(const Environment& env, Archive* arch) const
{
    return std::make_unique<EvoGraph::OldExecutionEngine>(env, arch);
}