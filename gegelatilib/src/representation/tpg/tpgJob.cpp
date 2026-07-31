

#include "representation/tpg/tpgJob.h"

Representation::TPG::TPGArchive& Representation::TPG::TPGJob::getArchive() const
{
    return *this->archive;
}