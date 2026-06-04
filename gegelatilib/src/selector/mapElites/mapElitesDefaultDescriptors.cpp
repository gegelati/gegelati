/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
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

#include "selector/mapElites/mapElitesDefaultDescriptors.h"

void Selector::MapElites::DefaultDescriptors::ActionValues::initDescriptor(
    const TPG::TPGGraph& graph,
    const Learn::LearningEnvironment& learningEnvironment)
{
    this->init = true;
    this->minRange = 0.0;
    this->maxRange = 1.0;
    this->nbDescriptors = learningEnvironment.getNbActions();
}

std::string Selector::MapElites::DefaultDescriptors::ActionValues::getName()
    const
{
    return "ActionValues";
}

void Selector::MapElites::DefaultDescriptors::ActionValues::extractMetricsStep(
    std::vector<double>& metrics, const TPG::TPGVertex* agent,
    std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment) const
{
    for (size_t idx = 0; idx < nbDescriptors; idx++) {
        metrics[idx] += std::abs(actionValues[idx]);
    }
}

void Selector::MapElites::DefaultDescriptors::ActionValues::
    extractMetricsEpisode(
        std::vector<double>& metrics, const TPG::TPGVertex* agent,
        size_t nbStepsExecuted,
        const Learn::LearningEnvironment& learningEnvironment) const
{
    for (size_t idx = 0; idx < nbDescriptors; idx++) {
        metrics[idx] /= nbStepsExecuted;
    }
}
