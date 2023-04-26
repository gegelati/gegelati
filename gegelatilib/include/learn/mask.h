#ifndef GEGELATI_MASK_H
#define GEGELATI_MASK_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <numeric>

#include "mutator/rng.h"

#define THRESHOLD 0.2

/*
 *  mask_t is used to manage the mask structure
 */
using mask_t = std::vector<std::vector<double>>;

/*
 *  The Mask class is a singleton that permit to access to a unique instance
 *  of the mask.
 */
class Mask
{
  public:

    /*
     *  \brief getInstance is the getter for the unique instance of the mask
     */
    static Mask& getInstance()
    {
        static Mask instance;
        return instance;
    }

    /*
     *  \brief updateMask update the mask content, need to be call once each generation
     *
     *  \param[in] mask_index is used to select what mask to use
     */
    void updateMask(uint64_t mask_index);                          // Allow to update the mask content (the log data shall be added to parameters)

    /*
     *  \brief  getIdx return a random index according to the statistic repartition
     *          in the mask content
     *
     *  \param[in] mask_index is used to select what mask to use
     *  \param[in] rng Random Number Generator used in the mutation process.
     */
    std::vector<int> getIdx(Mutator::RNG rng, uint64_t mask_index);                  // Return a random index according to the mask statistic repartition

    /*
     *  \brief init is used to initialize the masks content
     *
     *  \param[in] masks is the initial mask that will be used
     */
    void init(std::vector<mask_t> &masks);                                // Mutator in parameters, and maybe a data sample to make the mask size be coherent

    /*
     *  \brief getSize return the 2d size of the selected mask
     *
     *  \param[in] mask_index is used to select what mask to use
     */
    std::vector<int> getSize(uint64_t mask_index); // Mask dimensions

  private:
    // Needed to be a singleton
    Mask() = default;

    // The structure that contain all the different masks
    std::vector<mask_t> all_masks;

    //Mask(Mask const&);
    //void operator=(Mask const&);

  public:
    Mask(Mask const&)               = delete;
    void operator=(Mask const&)     = delete;
};


#endif // GEGELATI_MASK_H
