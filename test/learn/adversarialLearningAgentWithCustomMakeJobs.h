/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#ifndef ADVERSARIAL_LEARNING_AGENT_WITH_CUSTOM_MAKE_JOBS_H
#define ADVERSARIAL_LEARNING_AGENT_WITH_CUSTOM_MAKE_JOBS_H

#include "learn/adversarialLearningAgent.h"

// Class only used for testing purpose : we create a custom jobs organization
class AdversarialLearningAgentWithCustomMakeJobs
    : public Learn::AdversarialLearningAgent
{
  public:
    AdversarialLearningAgentWithCustomMakeJobs(Learn::LearningEnvironment& le,
                                               const ::Instructions::Set& iSet,
                                               Learn::LearningParameters& p)
        : Learn::AdversarialLearningAgent(le, iSet, p, 3)
    {
    }

  protected:
    // Warning : this override guess there are at least 3 roots in the tpg graph
    // It creates the following job :
    // root0-root1-root2
    // root0-root2-root1
    // root0-root1-root2
    // root1-root0-root2
    // It means root0 is 3 times at pos 0 and 1 time at pos 1
    // root 1 is 1 time at pos 0 2 times at pos 1 and 1 time at pos 3
    // root 2 is 1 time at pos 1 and 3 times at pos 3
    std::queue<std::shared_ptr<Learn::Job>> makeJobs(
        Learn::LearningMode mode, TPG::TPGGraph* tpgGraph = nullptr) override
    {
        // sets the tpg to the Learning Agent's one if no one was specified
        std::queue<std::shared_ptr<Learn::Job>> jobs;

        auto roots = tpg->getRootVertices();

        uint64_t archiveSeed;
        archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);

        auto job =
            std::make_shared<Learn::AdversarialJob>(Learn::AdversarialJob(
                {roots[0], roots[1], roots[2]}, archiveSeed, 0));
        jobs.push(job);

        archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);
        auto job2 =
            std::make_shared<Learn::AdversarialJob>(Learn::AdversarialJob(
                {roots[0], roots[2], roots[1]}, archiveSeed, 1));
        jobs.push(job2);

        archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);
        auto job3 =
            std::make_shared<Learn::AdversarialJob>(Learn::AdversarialJob(
                {roots[0], roots[1], roots[2]}, archiveSeed, 2));
        jobs.push(job3);

        archiveSeed = this->rng.getUnsignedInt64(0, UINT64_MAX);
        auto job4 =
            std::make_shared<Learn::AdversarialJob>(Learn::AdversarialJob(
                {roots[1], roots[0], roots[2]}, archiveSeed, 3));
        jobs.push(job4);

        return jobs;
    }
};

#endif
