
#include "algorithm/lgp/lgpAlgorithm.h"


std::vector<double> Algorithm::LGP::LGPAlgorithm::executeAgent(std::shared_ptr<const Agent> agent) const
{
    
}



 std::shared_ptr<const Environment> Algorithm::LGP::LGPAlgorithm::getEnvironment() const
{
    return this->env;
}

std::shared_ptr<const Archive> Algorithm::LGP::LGPAlgorithm::getArchive() const
{
    return this->archive;
}