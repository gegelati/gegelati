

#include "algorithm/tpg/tpgJob.h"

Archive& Algorithm::TPG::TPGJob::getArchive() const
{
    return *this->archive;
}