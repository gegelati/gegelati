//
// Created by asimonu on 06/07/2020.
//

#ifndef PARALLELJOB_H
#define PARALLELJOB_H

#include <cstdint>

#include "learn/job.h"

namespace Learn {
    class ParallelJob : public Job
    {
      private:
        const uint64_t idx;

        const uint64_t archiveSeed;

      public:
        ParallelJob() = delete;

        ParallelJob(uint64_t idx, uint64_t archiveSeed,
                    std::initializer_list<const TPG::TPGVertex*> roots)
            : Job(roots), archiveSeed(archiveSeed), idx(idx)
        {
        }

        uint64_t getIdx()
        {
            return idx;
        }

        uint64_t getArchiveSeed()
        {
            return archiveSeed;
        }
    };

} // namespace Learn

#endif
