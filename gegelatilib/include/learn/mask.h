#ifndef GEGELATI_MASK_H
#define GEGELATI_MASK_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <numeric>

class Mask
{
  public:
    static Mask& getInstance()
    {
        static Mask instance;
        return instance;
    }

    void updateMask();              // Allow to update the mask content (the log data shall be added to parameters)
    std::vector<int> getIdx();      // Return a random index according to the mask statistic repartition
    void init();                    // Mutator in parameters, and maybe a data sample to make the mask size be coherent

  private:
    Mask() = default;

    std::vector<std::vector<double>> _mask;    // Mask content | Statistic repartition

    //Mask(Mask const&);
    //void operator=(Mask const&);

  public:
    Mask(Mask const&)               = delete;
    void operator=(Mask const&)     = delete;
};


#endif // GEGELATI_MASK_H
