
#ifndef MAP_ELITES_DESCRIPTORS_H
#define MAP_ELITES_DESCRIPTORS_H

#include "tpg/tpgGraph.h"
#include "learn/learningEnvironment.h"


namespace Selector {
    namespace MapElites {


        class MapElitesDescriptor
        {
            void extractFeature(TPG::TPGVertex root);
        };

    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_DESCRIPTORS_H