
#include "algorithm/lgp/lgpAlgorithm.h"


std::vector<double> Algorithm::LGP::LGPAlgorithm::executeAgent(std::shared_ptr<const Agent> agent) const
{
    
}



const Environment& Algorithm::LGP::LGPAlgorithm::getEnvironment() const
{
    return this->env;
}

const Archive& Algorithm::LGP::LGPAlgorithm::getArchive() const
{
    return this->archive;
}