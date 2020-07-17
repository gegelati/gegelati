/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#include <vector>
#include <cstdint>

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
         */
        AdversarialJob(std::initializer_list<const TPG::TPGVertex*> roots,
            uint64_t archiveSeed=0, uint64_t idx=0)
                : roots(roots), Job(nullptr, archiveSeed,idx)
        {
        }

        /**
         * \brief Adds a root to this job and updates the size of the job.
         *
         * @param[in] root The root that will be added to this job.
         */
        void addRoot(const TPG::TPGVertex* root)
        {
            roots.emplace_back(root);
        }

        /**
         * \brief Getter of the number of roots.
         *
         * @return The number of roots in this job.
         */
        [[nodiscard]] size_t getSize() const
        {
            return roots.size();
        }

        /**
         * \brief Getter of the roots.
         *
         * @return The vector containing the roots of the job.
         */
        [[nodiscard]] std::vector<const TPG::TPGVertex*> getRoots() const
        {
            return roots;
        }

        /**
        * \brief Getter of the first root.
        *
        * @return The first root embedded by the job.
        */
        const TPG::TPGVertex* getRoot() const override
        {
            return roots[0];
        }

        /**
         * \brief Getter of a single root in the list.
         *
         * @param[in] i The wanted index in the list of roots.
         * @return The root found at index i.
         */
        const TPG::TPGVertex* operator[](int i) const
        {
            return roots[i];
        }
    };
} // namespace Learn

#endif
