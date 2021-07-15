#ifndef GEGELATI_TPGABSTRACTENGINE_H
#define GEGELATI_TPGABSTRACTENGINE_H

#include "program/program.h"
#include "tpgGraph.h"

namespace TPG {
    /**
     * \brief Abstract Class in charge of managing maps to give a unique ID
     * for vertex and a program of a TPGGraph.
     *
     */
    class TPGAbstractEngine
    {

      protected:
        /**
         * \brief Reference to the TPGGraph whose content will be used to fill
         * the maps.
         */
        const TPG::TPGGraph& tpg;

        /**
         * \brief Map associating pointers to Program to an integer ID.
         *
         * In case the TPGAbstractEngine is used to export multiple TPGGraph,
         * this map is used to ensure that a given Program will always be
         * associated to the same integer identifier in all exported files.
         */
        std::map<const Program::Program*, uint64_t> programID;

        /**
         * \brief Integer number used to associate a unique
         * integer identifier to each new Program.
         *
         * In case the TPGAbstractEngine is used to export multiple TPGGraph, a
         * Program that was already printed in previous export will keep its ID.
         */
        uint64_t nbPrograms = 0;

        /**
         * \brief Map associating pointers to TPGVertex to an integer ID.
         *
         * In case the TPGAbstractEngine is used to export multiple TPGGraph,
         * this map is used to ensure that a given TPGVertex will always be
         * associated to the same integer identifier in all exported files.
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
         * \brief Integer number used during export to associate a unique
         * integer identifier to each TPGAction.
         *
         * Identifier associated to TPGAction are NOT preserved during multiple
         * printing of a TPGGraph.
         */
        uint64_t nbActions;

        /**
         * \brief constructor for the abstract engine
         *
         * \param[in] tpg const reference to the graph whose content will be
         * used to fill the maps of IDs  (vertex and program).
         */

        TPGAbstractEngine(const TPG::TPGGraph& tpg) : tpg{tpg} {};

      public:
        /**
         * \brief Method for finding the unique identifier associated to a given
         * Program.
         *
         * Using the programID map, this method retrieves the integer identifier
         * associated to the given Program. If no identifier exists for this
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
} // namespace TPG
#endif // GEGELATI_TPGABSTRACTENGINE_H
