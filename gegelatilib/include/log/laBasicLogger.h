/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#ifndef LA_BASIC_LOGGER_H
#define LA_BASIC_LOGGER_H

#include <iomanip>

#include "log/laLogger.h"

namespace Log {

    /**
     * \brief Basic logger that will display some useful information
     *
     * The information logged by this LALogger are generation number, nb of
     * vertices, min, mean, avg score of this generation and to finish some
     * timing. Everything is logged like a tab with regularly spaced columns.
     */
    class LABasicLogger : public LALogger
    {
      private:
        /**
         * Width of columns when logging values.
         */
        int colWidth = 9;

        /**
         * \brief Logs the min, avg and max score of the generation.
         *
         * This method is used by the eval and valid callback as
         * they both have the same input and want to log the same elements
         * (min, avg max).
         */
        void logResults(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                      const TPG::TPGVertex*>& results);

      public:
        /**
         * \brief Same constructor as LaLogger. Default output is cout.
         *
         * \param[in] la LearningAgent whose information will be logged by the
         * LABasicLogger.
         * \param[in] out The output stream the logger will send
         * elements to.
         */
        explicit LABasicLogger(Learn::LearningAgent& la,
                               std::ostream& out = std::cout)
            : LALogger(la, out)
        {
            // fixing float precision
            *this << std::setprecision(2) << std::fixed << std::right;
            this->logHeader();
        }

        /**
         * Inherited via LaLogger
         *
         * \brief Logs the header (column names) of the tab that will be logged.
         */
        virtual void logHeader() override;

        /**
         * Inherited via LALogger.
         *
         * \brief Logs the generation of training.
         *
         * \param[in] generationNumber The number of the current
         * generation.
         */
        virtual void logNewGeneration(uint64_t& generationNumber) override;

        /**
         * Inherited via LALogger.
         *
         * \brief Logs the vertices nb of the tpg.
         */
        virtual void logAfterPopulateTPG() override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Logs the min, avg and max score of the generation.
         *
         * If doValidation is true, it only updates eval time.
         * The method logResults will be called in order to log
         * statistics about results (method shared with logAfterValidate).
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
        virtual void logAfterDecimate() override{
            // nothing to log
        };

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
         * \brief Logs the eval, valid (if doValidation is true)
         * and total running time.
         */
        virtual void logEndOfTraining() override;
    };

} // namespace Log

#endif
