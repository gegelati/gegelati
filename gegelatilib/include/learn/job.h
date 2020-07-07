//
// Created by asimonu on 03/07/2020.
//

#ifndef JOB_H
#define JOB_H

#include <stddef.h>
#include <tpg/tpgVertex.h>
#include <vector>

namespace Learn {
    class Job
    {
      protected:
        /**
        * The number of roots contained in the job.
        */
        size_t size;

        /**
        * The roots contained in the job. e.g. 2 roots that
        * will compete together in a 1vs1 game.
        */
        std::vector<const TPG::TPGVertex*> roots;

        /**
        * Index associated to this job.
        */
        const uint64_t idx;

        /**
        * Seed that will be used to randomize archive.
        */
        const uint64_t archiveSeed;

      public:
        /// Deleted default constructor.
        Job() = delete;

        /**
         * \brief Simple job constructor without archive seed or index, to be
         * used without parallelization.
         *
         * @param[in] roots The roots that will be encapsulated into the job.
         */
        Job(std::initializer_list<const TPG::TPGVertex*> roots)
            : roots(roots), size(roots.size()), archiveSeed(0),idx(0)
        {
        }

        /**
        * \brief Constructor enabling to store elements in the job so that the
        * Learning Agents will be able to use them later.
        *
        * @param idx The index of this job.
        * @param archiveSeed The archive seed that will be used with this job.
        * @param roots The roots that will be encapsulated into the job.
        */
        Job(uint64_t idx, uint64_t archiveSeed,
            std::initializer_list<const TPG::TPGVertex*> roots)
            : roots(roots), size(roots.size()), archiveSeed(archiveSeed),
              idx(idx)
        {
        }

        /**
         * \brief Getter of index.
         *
         * @return The index of the job.
         */
        uint64_t getIdx()
        {
            return idx;
        }

        /**
         * \brief Getter of archiveSeed.
         *
         * @return The archive seed of the job.
         */
        uint64_t getArchiveSeed()
        {
            return archiveSeed;
        }

        /**
        * \brief Getter of the number of roots.
        *
        * @return The number of roots in this job.
        */
        [[nodiscard]] size_t getSize() const
        {
            return size;
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
}

#endif
