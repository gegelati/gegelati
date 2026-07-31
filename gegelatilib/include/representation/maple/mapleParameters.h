/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2023)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (MAPLEs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef MAPLE_PARAMETERS_H
#define MAPLE_PARAMETERS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace Representation::Maple {
    /**
     * \brief Structure holding all parameters affecting maple.
     */
    typedef struct MapleParameters
    {

        /// JSon comment
        inline static const std::string pChangeActionClassComment =
            "//  Probablity of changing the action edge by any other action "
            "edge in the graph\n"
            "// \"pChangeActionClass\" : 0.1, // Default value";
        ///  Probablity of changing the action edge by any other action edge in
        ///  the graph
        double pChangeActionClass = 0.1;
        /// JSon comment
        inline static const std::string pActionEdgeDeletionComment =
            "// Probability of deleting an outgoing ActionEdge of a Action.\n"
            "// \"pActionEdgeDeletion\" : 0.7, // Default value";
        /// Probability of deleting an outgoing Edge of a Team.
        double pActionEdgeDeletion = 0.7;

        /// JSon comment
        inline static const std::string pActionEdgeAdditionComment =
            "// Probability of adding an outgoing Action Edge to a Action.\n"
            "// \"pActionEdgeAddition\" : 0.7, // Default value";
        /// Probability of adding an outgoing Edge to a Team.
        double pActionEdgeAddition = 0.7;

        /// JSon comment
        inline static const std::string pMutateActionProgramComment =
            "//  Probablity of mutation the program on the action edge\n"
            "// \"pMutateActionProgram\" : 0.1, // Default value";
        ///  Probablity of mutation the program on the action edge
        double pMutateActionProgram = 0.1;

        /// JSon comment
        inline static const std::string pSwapActionProgramComment =
            "//  Probablity of swapping two action edge within the same action "
            "vertex\n"
            "// \"pSwapActionProgram\" : 0.1, // Default value";
        ///  Probablity of swapping two action edge within the same action
        ///  vertex
        double pSwapActionProgram = 0.1;

        /// JSon comment
        inline static const std::string nbActionEdgeInitComment =
            "// Number of edge per action vertex at initialisation\n"
            "// \"nbActionEdgeInit\" : 1, // Default value";
        /// Init each team with one edge per action
        size_t nbActionEdgeInit = 1;

        /// JSon comment
        inline static const std::string pCrossAgentsComment =
            "// Probability of doing a crossover with two MAPLE agents (edge "
            "crossover by default).\n"
            "// \"pCrossAgents\" : 0.0, // Default value";
        /// Probability of doing a crossover with two MAPLE agents (edge
        /// crossover by default).
        double pCrossAgents = 0.0;

        /// JSon comment
        inline static const std::string pCrossProgramsComment =
            "// When doing a crossover with two MAPLE agents, probability to "
            "do a program crossover instead of an edge crossover.\n"
            "// \"pCrossPrograms\" : 0.0, // Default value";
        /// When doing a crossover with two MAPLE agents, probability to do a
        /// program crossover instead of an edge crossover.
        double pCrossPrograms = 0.0;

    } MapleParameters;

} // namespace Representation

#endif
