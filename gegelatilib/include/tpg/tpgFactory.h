#ifndef TPG_FACTORY_H
#define TPG_FACTORY_H

#include <memory>

#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgTeam.h"

namespace TPG {

    // Declare the TPGGraph class to be used as a parameter.
    class TPGGraph;

    /**
     * \brief Factory for creating all elements constituting a TPG.
     *
     * Using the factory design pattern, this class enables the creation of
     * all elements composing a TPGGraph:
     * - TPGGraph
     * - TPGTeam
     * - TPGAction
     * - TPGVertex
     *
     * This implementation returns the default type for each kind of element.
     */
    class TPGFactory
    {

      public:
        ///  Default virtual destructor.
        virtual ~TPGFactory() = default;

        /**
         * \brief Create a TPGGraph with this TPGFactory.
         *
         * \param[in] env Environment used to build the TPGGraph.
         */
        virtual std::shared_ptr<TPGGraph> createTPGGraph(
            const Environment& env) const;

        /**
         * \brief Create a TPGTeam for a TPGGraph.
         *
         * This method allocates and returns a new TPGTeam.
         */
        virtual TPGTeam* createTPGTeam() const;

        /**
         * \brief Create a TPGAction for a TPGGraph.
         *
         * This method allocates and returns a new TPGAction.
         *
         * \param[in] id integer stored as the actionID of the TPGAction.
         */
        virtual TPGAction* createTPGAction(const uint64_t id) const;

        /**
         * \brief Create a TPGEdge for a TPGGraph.
         *
         * This method allocates and returns a new TPGEdge.
         * The TPGEdge is returned as a unique_ptr.
         *
         * \param[in] src pointer to the source TPGVertex of the edge.
         * \param[in] dest pointer to the destination TPGVertex of the edge.
         * \param[in] prog the shared pointer to the Program associated to the
         *            edge.
         */
        virtual std::unique_ptr<TPGEdge> createTPGEdge(
            const TPGVertex* src, const TPGVertex* dest,
            const std::shared_ptr<Program::Program> prog) const;
    };
} // namespace TPG

#endif // !TPG_GRAPH_ELEMENT_FACTORY_H
