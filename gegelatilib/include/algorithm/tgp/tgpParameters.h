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

#ifndef TGP_PARAMETERS_H
#define TGP_PARAMETERS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace Algorithm::TGP {
    /**
     * \brief Structure holding all parameters affecting mutations of TGP.
     */
    typedef struct TGPParameters
    {

        /// JSon comment
        inline static const std::string maxDepthComment =
            "// Maximum depth that the graph can reach.\n"
            "// \"maxDepth\" : 5, // Default value";
        /// Maximum depth that the graph can reach.
        size_t maxDepth = 5;

        /// JSon comment
        inline static const std::string maxInitDepthComment =
            "// Maximum depth that the graph can reach at initialisation.\n"
            "// \"maxInitDepth\" : 3, // Default value";
        /// Maximum depth that the graph can reach.
        size_t maxInitDepth = 3;

        /// JSon comment
        inline static const std::string maxNbEdgePerNodeComment =
            "// Maximum number of edges per node, we strongly advice to let it at 2 to avoid complexity explosion.\n"
            "// \"maxNbEdgePerNode\" : 2, // Default value";
        /// Maximum number of edges per node, we strongly advice to let it at 2 to avoid complexity explosion.
        size_t maxNbEdgePerNode = 2;

    } TGPParameters;
} // namespace Algorithm

#endif
