/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2026) :
 *
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2026)
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

#include "selector/timingSelectionMetrics.h"

void Selector::TimingSelectionMetrics::extractMetricsStep(
    const TPG::TPGVertex* agent, const std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment)
{
    wrapped->extractMetricsStep(agent, actionValues, learningEnvironment);
};

void Selector::TimingSelectionMetrics::extractMetricsEpisodeWithTiming(
    const TPG::TPGVertex* agent, size_t nbStepsExecuted,
    const Learn::LearningEnvironment& learningEnvironment,
    double agentTimeEpisode, double leTimeEpisode)
{
    wrapped->extractMetricsEpisode(agent, nbStepsExecuted, learningEnvironment);

    nbActions += nbStepsExecuted;
    agentTime += agentTimeEpisode;
    leTime += leTimeEpisode;
}

void Selector::TimingSelectionMetrics::weightedSum(
    std::shared_ptr<SelectionMetrics> other, size_t nbEvaluation,
    size_t nbEvaluationOther)
{
    if (auto tother = dynamic_cast<TimingSelectionMetrics*>(other.get())) {
        wrapped->weightedSum(tother->wrapped, nbEvaluation, nbEvaluationOther);

        agentTime = SelectionMetrics::weightedSum(
            agentTime, tother->agentTime, nbEvaluation, nbEvaluationOther);
        leTime = SelectionMetrics::weightedSum(leTime, tother->leTime,
                                               nbEvaluation, nbEvaluationOther);
        nbActions = SelectionMetrics::weightedSum(
            nbActions, tother->nbActions, nbEvaluation, nbEvaluationOther);
    }
    else {
        throw std::runtime_error("Type mismatch between SelectionMetrics.");
    }
}

bool Selector::operator<(std::shared_ptr<TimingSelectionMetrics> a,
                         std::shared_ptr<TimingSelectionMetrics> b)
{
    return *a < *b;
}

bool Selector::operator<(std::shared_ptr<SelectionMetrics> a,
                         std::shared_ptr<TimingSelectionMetrics> b)
{
    return *a < *b;
}

bool Selector::operator<(std::shared_ptr<TimingSelectionMetrics> a,
                         std::shared_ptr<SelectionMetrics> b)
{
    return *a < *b;
}
