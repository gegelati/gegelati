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

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

namespace Log {

    /**
     * Basic LALogger for logging generic training information.
     */
    class Logger
    {
      private:
        /**
         * Output stream where all what is logged is put.
         */
        std::ostream* out;

      public:
        /**
         * \brief Constructor initializing a specific output. Default is cout.
         *
         * \param[in] out The output stream the logger will send elements to.
         */
        explicit Logger(std::ostream& out = std::cout) : out(&out){};

        /**
         * \brief << operator to manipulate stream and enter stream-specific
         * elements (like std::endl).
         *
         * \param[in] manip The element that will be added.
         * \return The samme logger to be able to stream several things
         * (e.g. logger<<elA<<elB).
         */
        Logger operator<<(std::ostream& (*manip)(std::ostream&));

        /**
         * \brief << operator allowing to log elements that ostream actually
         * accepts (char, int...).
         *
         * \tparam T The type of element that will be logged.
         * \param[in] val The element that will be logged.
         * \return The same logger to be able to stream several things
         * (e.g. logger<<elA<<elB).
         */
        template <typename T> Logger operator<<(const T& val)
        {
            *out << val;

            // flushes the buffer, useful especially with ofstream where without
            // that, nothing will be printed until close
            out->flush();

            return *this;
        }
    };
} // namespace Log
#endif
