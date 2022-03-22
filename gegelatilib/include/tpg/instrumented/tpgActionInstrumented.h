
#ifndef TPG_ACTION_INSTRUMENTED_H
#define TPG_ACTION_INSTRUMENTED_H

#include "tpg/instrumented/tpgVertexInstrumentation.h"
#include "tpg/tpgAction.h"

namespace TPG {

    /**
     * \brief Instrumented TPGAction
     */
    class TPGActionInstrumented : public TPG::TPGAction,
                                  public TPG::TPGVertexInstrumentation
    {
      public:
        /// Main constructor for TPGActionInstrumented.
        /// see TPGAction constructor for more details.
        TPGActionInstrumented(const uint64_t id) : TPGAction(id)
        {
        }
    };
} // namespace TPG

#endif