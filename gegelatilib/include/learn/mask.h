#ifndef GEGELATI_MASK_H
#define GEGELATI_MASK_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <numeric>

#include "mutator/rng.h"

#define THRESHOLD 0.2

using mask_t = std::vector<std::vector<double>>;

class Mask
{
  public:
    static Mask& getInstance()
    {
        static Mask instance;
        return instance;
    }

    void updateMask(uint64_t mask_index);                          // Allow to update the mask content (the log data shall be added to parameters)
    std::vector<int> getIdx(Mutator::RNG rng, uint64_t mask_index);                  // Return a random index according to the mask statistic repartition
    void init(std::vector<mask_t> &masks);                                // Mutator in parameters, and maybe a data sample to make the mask size be coherent
    std::vector<int> getSize(uint64_t mask_index); // Mask dimensions

  private:
    Mask() = default;

    std::vector<mask_t> all_masks;

    //Mask(Mask const&);
    //void operator=(Mask const&);

  public:
    Mask(Mask const&)               = delete;
    void operator=(Mask const&)     = delete;
};


#endif // GEGELATI_MASK_H
