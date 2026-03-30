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

#ifndef LGP_PARAMETERS_H
#define LGP_PARAMETERS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace Algorithm::LGP {
    /**
     * \brief Structure holding all parameters affecting mutations of LGP.
     */
    typedef struct LGPParameters
    {

        /// JSon comment
        inline static const std::string nbRegistersComment =
            "// Number of registers for the Program execution.\n"
            "// \"nbRegisters\" : 8, // Default value";
        /// Number of registers for the Program execution
        size_t nbRegisters = 8;

        /// JSon comment
        inline static const std::string nbProgramConstantComment =
            "// Number of Constant available in each Program.\n"
            "// \"nbProgramConstant\" : 0, // Default value";
        /// Number of Constants available in a program.
        size_t nbProgramConstant = 0;

        /// JSon comment
        inline static const std::string maxProgramSizeComment =
            "// Maximum number of Line within the Program of the TPG.\n"
            "// \"maxProgramSize\" : 96, // Default value";
        /// Maximum number of Line within the Program of the Graph.
        size_t maxProgramSize = 96;

        /// JSon comment
        inline static const std::string initMinProgramSizeComment =
            "// Initial minimum number of Line within the Program of the "
            "Graph.\n"
            "// \"initMinProgramSize\" : 1, // Default value";
        /// Initial minimum number of Line within the Program of the Graph.
        size_t initMinProgramSize = 1;

        /// JSon comment
        inline static const std::string initMaxProgramSizeComment =
            "// Initial maximum number of Line within the Program of the "
            "Graph.\n"
            "// \"initMaxProgramSize\" : 10, // Default value";
        /// Initial maximum number of Line within the Program of the Graph.
        size_t initMaxProgramSize = 10;

        /// JSon comment
        inline static const std::string pDeleteComment =
            "// Probability of deleting a line of the Program.\n"
            "// \"pDelete\" : 0.5, // Default value";
        /// Probability of deleting a line of the Program.
        double pDelete = 0.5;

        /// JSon comment
        inline static const std::string pAddComment =
            "// Probability of inserting a line in the Program.\n"
            "// \"pAdd\" : 0.5, // Default value";
        /// Probability of inserting a line in the Program
        double pAdd = 0.5;

        /// JSon comment
        inline static const std::string pMutateComment =
            "// Probability of altering a line of the Program.\n"
            "// \"pMutate\" : 1.0, // Default value";
        /// Probability of altering a line of the Program.
        double pMutate = 1.0;

        /// JSon comment
        inline static const std::string pSwapComment =
            "// Probability of swapping two lines of the Program.\n"
            "// \"pSwap\" : 1.0, // Default value";
        /// Probability of swapping two lines of the Program.
        double pSwap = 1.0;

        /// JSon comment
        inline static const std::string pConstantMutationComment =
            "// Probability of each constant to be mutated.\n"
            "// \"pConstantMutation\" : 0.5, // Default value";
        /// Probability of each constant to be mutated
        double pConstantMutation = 0.5;

        /// JSon comment
        inline static const std::string pNewProgramComment =
            "// Probability of creating a new program.\n"
            "// \"pNewProgram\" : 0.0, // Default value";
        /// Probability of creating a new program
        double pNewProgram = 0.0;

        /// JSon comment
        inline static const std::string minConstValueComment =
            "// Minimum constant value possible.\n"
            "// \"minConstValue\" : -1.0, // Default value";
        /// Minimum constant value possible
        double minConstValue = -1.0;

        /// JSon comment
        inline static const std::string maxConstValueComment =
            "// Maximum constant value possible.\n"
            "// \"maxConstValue\" : 1.0, // Default value";
        /// Maximum constant value possible
        double maxConstValue = 1.0;
        
        /// JSon comment
        inline static const std::string pMutateOutputComment =
            "// Probability of mutating an output index.\n"
            "// \"pMutateOutput\" : 0.0, // Default value";
        /// Probability of mutating an output index.
        double pMutateOutput = 0.0;

    } LGPParameters;
} // namespace Algorithm

#endif
