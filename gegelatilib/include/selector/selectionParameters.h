/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2023)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
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

#ifndef SELECTION_PARAMETERS_H
#define SELECTION_PARAMETERS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace Selector {
    /**
     * \brief Structure holding all parameters affecting mutations of Truncation
     * Selection
     */
    typedef struct TruncationParameters
    {
        /// JSon comment
        inline static const std::string ratioDeletedRootsComment =
            "// Percentage of deleted (and regenerated) root Vertex at each "
            "generation.\n"
            "// \"ratioDeletedRoots\" : 0.5, // Default value";
        /// Percentage of deleted (and regenerated) root Vertex at each
        /// generation.
        double ratioDeletedRoots = 0.5;

    } TruncationParameters;

    /**
     * \brief Structure holding all parameters affecting mutations of Truncation
     * Selection
     */
    typedef struct TournamentParameters
    {

        /// JSon comment
        inline static const std::string sizeTournamentComment =
            "// Size of tournament if tournament selection is used.\n"
            "// \"sizeTournament\" : 5, // Default value";
        /// Size of tournament if tournament selection is used.
        uint64_t sizeTournament = 5;

        /// JSon comment
        inline static const std::string ratioSavedRootsComment =
            "// Percentage of root Vertex saved from the tournament at each "
            "generation.\n"
            "// \"ratioSavedRoots\" : 0.05, // Default value";
        /// Percentage of root Vertex saved from the tournament at each
        /// generation.
        double ratioSavedRoots = 0.05;

        /// JSon comment
        inline static const std::string areElitesReproductibleComment =
            "// Indicates if the elites are used in the reproduction process.\n"
            "// \"areElitesReproductible\" : false, // Default value";
        /// Indicates if the elites are used in the reproduction process.
        bool areElitesReproductible = false;

    } TournamentParameters;

    /**
     * \brief Structure holding all parameters affecting selection.
     */
    typedef struct SelectionParameters
    {

        /// JSon comment
        inline static const std::string selectionModeComment =
            "// Selection mode used. Available modes are:\n"
            "//   - \"truncation\"\n"
            "//   - \"tournament\"\n"
            "// \"_selectionMode\" : Truncation, // Default value";
        /// Selection mode used. Available mode are:
        ///   - "truncation"
        ///   - "tournament"
        std::string _selectionMode = "truncation";

        /// Parameters for TruncationSelector
        TruncationParameters truncation;
        /// Parameters for ProgramMutator
        TournamentParameters tournament;
    } SelectionParameters;
} // namespace Selector

#endif
