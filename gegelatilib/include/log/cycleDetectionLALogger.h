/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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

#include "laLogger.h"

namespace Log {

    /**
     * \brief Detector for connected cycles in TPGs
     *
     * This utility class implement a depth-first search algorithm for detecting
     * the presence of directed cyclic paths in TPGs.
     */
    class CycleDetectionLALogger : public LALogger
    {
      private:
        /// Control whether a message is printed when no loop is detected.
        bool logOnSuccess;

      public:
        /**
         * \brief Same constructor as LALogger. Default output is cerr.
         *
         * \param[in] la LearningAgent whose TPG will be analyzed by the
         * CycleDetectionLALogger.
         * \param[in] out The output stream the logger will send
         * elements to.
         * \param[in] logOnSuccess When true, the logger will log the absence of
         * cycles.
         */
        explicit CycleDetectionLALogger(Learn::LearningAgent& la,
                                        std::ostream& out = std::cerr,
                                        bool logOnSuccess = false)
            : LALogger(la, out), logOnSuccess(logOnSuccess)
        {
        }

        /**
         * Inherited via LALogger
         *
         * \brief Does nothing in this logger.
         */
        virtual void logHeader() override;

        /**
         * Inherited via LALogger.
         *
         * \brief Does nothing in this logger.
         */
        virtual void logNewGeneration(uint64_t& generationNumber) override;

        /**
         * Inherited via LALogger.
         *
         * \brief Checks the presence of directed cyclic path in the TPG. Loggs
         * an output if there is one.
         */
        virtual void logAfterPopulateTPG() override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Does nothing in this logger.
         *
         * \param[in] results scores of the evaluation.
         */
        virtual void logAfterEvaluate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Does nothing in this logger.
         */
        virtual void logAfterDecimate() override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Logs the min, avg and max score of the generation.
         *
         * If doValidation is true, no eval results are logged so that
         * the logger can only show validation results.
         *
         * \param[in] results scores of the validation.
         */
        virtual void logAfterValidate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) override;
        /**
         * Inherited via LaLogger
         *
         * \brief Does nothing in this logger.
         */
        virtual void logEndOfTraining() override;
    };
}; // namespace Log
