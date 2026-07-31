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

#include "representation/policyStats.h"
#include <algorithm>
#include <numeric>

void Representation::PolicyStats::clear()
{
    this->maxPolicyDepth = 0;
    this->nbDistinctTeams = 0;
    this->vertexPerDepthLevel.clear();
    this->nbUsagePerActionID.clear();
    this->nbUsePerTeam.clear();
    this->nbUsePerAction.clear();
}

Representation::PolicyStats& Representation::PolicyStats::getSubPolicyStats(uint64_t subRepresentationID) const
{
    auto it = this->subPolicyStats.find(subRepresentationID);
    if (it == this->subPolicyStats.end()) {
        throw std::invalid_argument("No sub policy stats found for representation " + subRepresentationID);
    }
    return *it->second;
}

void Representation::PolicyStats::analyzeVertex(const EvoGraph::Vertex& vertex, size_t depth)
{
    if (auto team = dynamic_cast<const EvoGraph::Team*>(&vertex)) {
        this->analyzeTeam(*team);
    }
    else if (auto action = dynamic_cast<const EvoGraph::Action*>(&vertex)) {
        this->analyzeAction(*action);
    }

    if(this->vertexPerDepthLevel.size() == depth){
        this->vertexPerDepthLevel.insert({depth, {}});
    }
    this->vertexPerDepthLevel.at(depth).insert(vertex);

    if(vertex.hasProgram()) {
        // Get the corresponding sub policy stats and analyze the policy of the program.
        this->getSubPolicyStats(vertex.getProgram().getRepresentationID()).analyzePolicy(vertex.getProgram());
    }

    for(const EvoGraph::Edge& edge : vertex.getOutgoingEdges()) {
        if(edge.hasProgram()) {
            // Get the corresponding sub policy stats and analyze the policy of the program.
            this->getSubPolicyStats(edge.getProgram().getRepresentationID()).analyzePolicy(edge.getProgram());
        }
        this->analyzeVertex(edge.getDestination(), depth + 1);
    }
}


void Representation::PolicyStats::analyzeTeam(const EvoGraph::Team& team)
{
    size_t nbUse = ++this->nbUsePerTeam[team];
    if (nbUse == 1) {
        this->nbDistinctTeams++;
        this->nbOutgoingEdgesPerTeam.push_back(team.getOutgoingEdges().size());
    }
}

void Representation::PolicyStats::analyzeAction(const EvoGraph::Action& action)
{
    this->nbUsePerAction[action]++;
    this->nbUsagePerActionID[action.getActionID()]++;
}

std::map<uint64_t, std::reference_wrapper<const Representation::PolicyStats>> Representation::PolicyStats::getAllSubPolicyStats() const
{
    std::map<uint64_t, std::reference_wrapper<const PolicyStats>> allSubPolicyStats;
    allSubPolicyStats.insert({this->representationID, *this});
    std::vector<std::reference_wrapper<const PolicyStats>> toVisit = {*this};
    while(!toVisit.empty()){
        const PolicyStats& current = toVisit.back();
        toVisit.pop_back();
        for(const auto& pair: current.subPolicyStats){
            allSubPolicyStats.insert({pair.first, std::ref(*pair.second)});
            toVisit.push_back(*pair.second);
        }
    }
    return allSubPolicyStats;
}

std::ostream& Representation::operator<<(std::ostream& os,
                              const Representation::PolicyStats& policyStats)
{
    auto sumVec = [](const std::vector<size_t>& vec) {
        return std::accumulate(vec.cbegin(), vec.cend(), (size_t)0);
    };

    os << "# PolicyStats" << std::endl;
    os << "## Topology info" << std::endl;
    os << "Teams:\t\t" << policyStats.nbDistinctTeams << std::endl;
    os << "Edges:\t\t" << sumVec(policyStats.nbOutgoingEdgesPerTeam)
       << std::endl;
    os << "Actions:\t" << policyStats.nbUsePerAction.size() << std::endl;

    os << "Stages\t\t" << policyStats.maxPolicyDepth << std::endl;
    os << "Vertex/stage:\t";
    for (auto& vertexPerStage : policyStats.vertexPerDepthLevel) {
        os << "{" << vertexPerStage.first << "," << vertexPerStage.second.size()
           << "} ";
    }
    os << std::endl;

    os << "Use/action:\t";
    if (!policyStats.nbUsagePerActionID.empty()) {
        os << (double)std::accumulate(
                  policyStats.nbUsagePerActionID.begin(),
                  policyStats.nbUsagePerActionID.end(), (size_t)0,
                  [](size_t accu, std::pair<size_t, size_t> val) {
                      return accu + val.second;
                  }) /
                  (double)policyStats.nbUsagePerActionID.size();
    }
    os << ": ";
    for (const auto& val : policyStats.nbUsagePerActionID) {
        os << "{" << val.first << "," << val.second << "} ";
    }
    os <<"\n\n"<< std::endl;

    auto allSubPolicyStats = policyStats.getAllSubPolicyStats();
    // Print specific policy stats
    for(const auto& pair: allSubPolicyStats){
        os << "## " << pair.second.get().representationName << ":" << pair.first << " Info"<<std::endl;
        os << pair.second.get().specificInfos();
        os <<"\n\n"<< std::endl;
    }

    return os;
}
