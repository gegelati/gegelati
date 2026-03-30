

#include "algorithm/tpg/tpgJob.h"

Algorithm::TPG::TPGArchive& Algorithm::TPG::TPGJob::getArchive() const
{
    return *this->archive;
}