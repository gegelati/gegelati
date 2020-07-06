//
// Created by asimonu on 03/07/2020.
//

#ifndef GEGELATI_JOB_H
#define GEGELATI_JOB_H

#include <stddef.h>
#include <tpg/tpgVertex.h>
#include <vector>

namespace Learn {
    class Job
    {
      private:
        size_t size;

        std::vector<const TPG::TPGVertex*> roots;

      public:
        Job() : size(0){}

        Job(std::initializer_list<const TPG::TPGVertex*> roots) : roots(roots), size(roots.size())
        {
        }

        bool isSingleRoot()
        {
            return size == 1;
        }

        [[nodiscard]] size_t getSize() const
        {
            return size;
        }

        [[nodiscard]] std::vector<const TPG::TPGVertex*> getRoots() const
        {
            return roots;
        }

        const TPG::TPGVertex* operator[](int i) const
        {
            return roots[i];
        }
    };
} // namespace Learn

#endif // GEGELATI_JOB_H
