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

#include "evoGraph/edge.h"

// Declaration of static edge ID Counter in local here because it creates error
// in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
uint64_t COUNT_EDGE_ID = 0;

uint64_t EvoGraph::Edge::incrementeCounter()
{
    return COUNT_EDGE_ID++;
}

uint64_t EvoGraph::Edge::getEdgeIDCounter()
{
    return COUNT_EDGE_ID;
}

void EvoGraph::Edge::resetEdgeIDCounter()
{
    COUNT_EDGE_ID = 0;
}

void EvoGraph::Edge::setEdgeID(uint64_t newID)
{
    this->edgeID = newID;

    // Update the ID counter if needed
    if (newID >= COUNT_EDGE_ID) {
        COUNT_EDGE_ID = newID + 1;
    }
}

std::shared_ptr<const EvoGraph::Vertex> EvoGraph::Edge::getSource() const
{
    return this->source;
}

void EvoGraph::Edge::setSource(std::shared_ptr<const EvoGraph::Vertex> newSource)
{
    this->source = newSource;
}

std::shared_ptr<const EvoGraph::Vertex> EvoGraph::Edge::getDestination() const
{
    return this->destination;
}

void EvoGraph::Edge::setDestination(std::shared_ptr<const EvoGraph::Vertex> newDestination)
{
    this->destination = newDestination;
}

uint64_t EvoGraph::Edge::getEdgeID() const
{
    return edgeID;
}

bool EvoGraph::operator<(const EvoGraph::Edge& a, const EvoGraph::Edge& b)
{
    return a.getEdgeID() < b.getEdgeID();
}