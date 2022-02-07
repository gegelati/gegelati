
#ifndef TPG_TEAM_INSTRUMENTED_H
#define TPG_TEAM_INSTRUMENTED_H

#include "tpg/instrumented/tpgVertexInstrumentation.h"
#include "tpg/tpgTeam.h"

namespace TPG {

    /**
     * \brief Instrumented TPGTeam
     */
    class TPGTeamInstrumented : public TPG::TPGTeam,
                                public TPG::TPGVertexInstrumentation
    {
    };
} // namespace TPG

#endif