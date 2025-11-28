/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2021)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef LEARNING_PARAMETERS_H
#define LEARNING_PARAMETERS_H

#include "mutator/mutationParameters.h"
#include "selector/selectionParameters.h"
#include <thread>

namespace Learn {
    /**
     * \brief Structure for simplifying the transmission of LearningParameters
     * to functions.
     *
     * When modifying this structure and its comments, you must also update the
     * functions in the ParameterParser namespace.
     */
    typedef struct LearningParameters
    {
        /// MutationParameters for controlling stochastic aspects of the
        /// learning process.
        Mutator::MutationParameters mutation;

        /// SelectionParameters for controlling selection aspects of the
        /// learning process.
        Selector::SelectionParameters selection;

        /// JSon comment
        inline static const std::string archiveSizeComment =
            "// Number of recordings held in the Archive.\n"
            "// \"archiveSize\" : 50, // Default value";
        /// Number of recordings held in the Archive.
        size_t archiveSize = 50;

        /// JSon comment
        inline static const std::string archivingProbabilityComment =
            "// Probability of archiving the result of each Program "
            "execution.\n"
            "// \"archivingProbability\" : 0.05, // Default value";
        /// Probability of archiving the result of each Program execution.
        double archivingProbability = 0.05;

        /// JSon comment
        inline static const std::string nbIterationsPerPolicyEvaluationComment =
            "// Number of evaluation of each root per generation.\n"
            "// \"nbIterationsPerPolicyEvaluation\" : 5, // Default value";
        /**
         * \brief Number of evaluation of each policy per generation.
         *
         * In LearningAgent and ParallelLearningAgent it is just the number of
         * times the evaluations are repeated (that can produce a more
         * representative result in non-deterministic environments).
         */
        uint64_t nbIterationsPerPolicyEvaluation = 5;

        /// JSon comment
        inline static const std::string nbIterationsPerPolicyValidationComment =
            "// Number of evaluation of each root per generation.\n"
            "// \"nbIterationsPerPolicyValidation\" : 5, // Default value";
        /**
         * \brief Number of evaluation of each policy per generation for
         * validation.
         */
        uint64_t nbIterationsPerPolicyValidation = 5;

        /// JSon comment
        inline static const std::string maxNbActionsPerEvalComment =
            "// Maximum number of actions performed on the learning "
            "environment during the\n"
            "// each evaluation of a root.\n"
            "// \"maxNbActionsPerEval\" : 1000, // Default value";
        /// Maximum number of actions performed on the learning environment
        /// during the each evaluation of a root
        uint64_t maxNbActionsPerEval = 1000;

        /// JSon comment
        inline static const std::string nbGenerationsComment =
            "// Number of generations of the training.\n"
            "// \"nbGenerations\" : 500, // Default value";
        /// Number of generations of the training.
        uint64_t nbGenerations = 500;

        /// JSon comment
        inline static const std::string maxNbEvaluationPerPolicyComment =
            "// Maximum number of times a given root is evaluated."
            "After this number is\n"
            "// reached, possibly after several generations, the score of the "
            "root will be\n"
            "// fixed, and no further evaluation will be done.\n"
            "// \"maxNbEvaluationPerPolicy\" : 1000, // Default value";
        /// Maximum number of times a given policy (i.e. a root Vertex) is
        /// evaluated.
        size_t maxNbEvaluationPerPolicy = 1000;

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
        inline static const std::string nbThreadsComment =
            "// [Only used in ParallelLearningAgent and child classes.]\n"
            "// Number of threads used for the training process.\n"
            "// When undefined in the json file, this parameter is "
            "automatically set to the\n"
            "// number of cores of the CPU.\n"
            "// /* \"nbThreads\" : 0,*/ // Commented by default";
        /**
         * \brief Number of threads (ParallelLearningAgent only)
         *
         * Integer parameter controlling the number of
         * threads used for parallel execution. Possible values are:
         *   - default :  Let the runtime decide using
         *         std::thread::hardware_concurrency().
         *   - `0` or `1`: Do not use parallelism.
         *   - `n > 1`: Set the number of threads explicitly.
         */
        size_t nbThreads = std::thread::hardware_concurrency();

        /// JSon comment
        inline static const std::string activationFunctionComment =
            "// string that indicate the activation function used for "
            "continuous actions \n"
            "// \"activationFunction\" : 'none', // Default value";
        /// string that indicate the activation function used for continuous
        /// actions
        std::string activationFunction = "none";

        /// JSon comment
        inline static const std::string doValidationComment =
            "// Boolean used to activate an evaluation of the surviving roots "
            "in validation\n"
            "// mode after the training at each generation.\n"
            "// \"doValidation\" : false, // Default value";
        /// Boolean set to true if the user wants a validation after each
        /// training, and false otherwise
        bool doValidation = false;

        /// JSon comment
        inline static const std::string stepValidationComment =
            "// Value to indicate how many generation between each validation "
            "step\n"
            "// \"stepValidation\" : 1, // Default value";
        /// Value to indicate how many generation between each validation step
        uint64_t stepValidation = 1;
    } LearningParameters;
}; // namespace Learn

#endif
