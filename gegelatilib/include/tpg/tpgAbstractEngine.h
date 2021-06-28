#ifndef GEGELATI_TPGABSTRACTENGINE_H
#define GEGELATI_TPGABSTRACTENGINE_H

#include "tpgGraph.h"
#include "program/program.h"

namespace TPG {
    /**
     * \brief Abstract Class in charge of managing maps to give a unique ID
     * for each of a TPGGraph
     *
     *
    */
    class TPGAbstractEngine
    {

      protected:

        /**
         * \brief Reference to the TPGGraph for which the code gen is generated.
         */
        const TPG::TPGGraph& tpg;

        /**
         * \brief Map associating pointers to Program to an integer ID.
         *
         */
        std::map<const Program::Program*, uint64_t> programID;

        /**
         * \brief Integer number used during export to associate a unique
         * integer identifier to each new Program.
         *
         * Using the programID map, a Program that was already printed in
         * previous export will keep its ID.
         */
        uint64_t nbPrograms = 0;

        /**
         * \brief Map associating pointers to TPGVertex to an integer ID.
         *
         * In case the TPGGraphDotExporter is used to export multiple TPGGraph,
         * this map is used to ensure that a given TPGVertex will always be
         * associated to the same integer identifier in all exported dot files.
         */
        std::map<const TPG::TPGVertex*, uint64_t> vertexID;

        /**
         * \brief Integer number used during export to associate a unique
         * integer identifier to each new TPGTeam.
         *
         * Using the VertexID map, a TPGTeam that was already printed in
         * previous export will keep its ID.
         */
        uint64_t nbVertex = 0;

        /**
         * \brief constructor
         * @param tpg
         */

        TPGAbstractEngine(const TPG::TPGGraph& tpg) : tpg{tpg}{};

      public:
        /**
         * \brief Method for finding the unique identifier associated to a given
         * Program.
         *
         * Using the programID map, this method retrieves the integer identifier
         * associated to the given Program. If not identifier exists for this
         * Program, a new one is created automatically and saved into the map.
         *
         * \param[in] prog a const reference to the Program whose integer
         *                    identifier is retrieved.
         * \param[out] id a pointer to an integer number, used to return the
         *                found identifier.
         * \return A boolean value indicating whether the returned ID is a new
         * one (true), or one found in the programID map (false).
         */

        bool findProgramID(const Program::Program& prog, uint64_t& id);

        /**
         * \brief Method for finding the unique identifier associated to a given
         * TPGVertex.
         *
         * Using the vertexID map, this method returns the integer identifier
         * associated to the given TPGVertex. If not identifier exists for this
         * TPGVertex, a new one is created automatically and saved into the map.
         *
         * \param[in] vertex a const reference to the TPGVertex whose integer
         *                    identifier is retrieved.
         * \return the integer identifier for the given TPGVertex.
         */

        uint64_t findVertexID(const TPG::TPGVertex& vertex);

    };
}
#endif // GEGELATI_TPGABSTRACTENGINE_H
