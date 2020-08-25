/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
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

#ifndef ADVERSARIAL_JOB_H
#define ADVERSARIAL_JOB_H

#include <cstdint>
#include <vector>

#include "learn/job.h"

namespace Learn {
    /**
     * \brief This class embeds roots for the simulations.
     *
     * The goal of the AdversarialJob is to contain several roots, so that each
     * job will match with one simulation/evaluation.
     * An adversarial learning agent will embed for example 2 roots per job
     * to simulate 2 agents activities (like a tic-tac-toe).
     */
    class AdversarialJob : public Job
    {
      protected:
        /**
         * The roots contained in the job. e.g. 2 roots that
         * will compete together in a 1vs1 game.
         */
        std::vector<const TPG::TPGVertex*> roots;

        /**
         * Position of the root that is to be evaluated.
         * Equal to -1 if there is no one.
         * That's typically useful if we have e.g. 4 champions
         * and a root to evaluate in the job : we can
         * skip the champions and just look at the score of
         * the root.
         *
         * See
         * AdversarialLearningAgent.evaluateAllRootsInParallelCompileResults()
         * for further understandings of its usage.
         */
        const int16_t posOfStudiedRoot;

      public:
        /// Deleted default constructor.
        AdversarialJob() = delete;

        /**
         * \brief Constructor enabling storing elements in the job so that the
         * Learning Agents will be able to use them later.
         *
         * @param[in] roots The roots that will be encapsulated into the job.
         * @param[in] archiveSeed The archive seed that will be used with this
         * job.
         * @param[in] idx The index of this job.
         * @param[in] posOfStudiedRoot The position of the root we will have
         * to check the score. Equal to -1 if all roots have to be checked.
         */
        AdversarialJob(std::initializer_list<const TPG::TPGVertex*> roots,
                       uint64_t archiveSeed = 0, uint64_t idx = 0,
                       int16_t posOfStudiedRoot = -1)
            : roots(roots), Job(nullptr, archiveSeed, idx),
              posOfStudiedRoot(posOfStudiedRoot)
        {
        }

        /**
         * \brief Adds a root to this job and updates the size of the job.
         *
         * @param[in] root The root that will be added to this job.
         */
        void addRoot(const TPG::TPGVertex* root);

        /**
         * \brief Getter of the number of roots.
         *
         * @return The number of roots in this job.
         */
        size_t getSize() const;

        /**
         * \brief Getter of the roots.
         *
         * @return The vector containing the roots of the job.
         */
        std::vector<const TPG::TPGVertex*> getRoots() const;

        /**
         * \brief Getter of the first root.
         *
         * @return The first root embedded by the job.
         */
        const TPG::TPGVertex* getRoot() const override;

        /**
         * \brief Getter of a single root in the list.
         *
         * @param[in] i The wanted index in the list of roots.
         * @return The root found at index i.
         */

        const TPG::TPGVertex* operator[](int i) const;

        /**
         * \brief Getter of the posOfStudiedRoot.
         *
         * @return The position of the evaluated root.
         */
        virtual const int16_t getPosOfStudiedRoot() const;
    };
} // namespace Learn

#endif
