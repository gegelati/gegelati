/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#ifndef LGP_EXECUTION_ENGINE_H
#define LGP_EXECUTION_ENGINE_H

#include <type_traits>

#include "data/primitiveTypeArray.h"
#include "data/untypedSharedPtr.h"
#include "algorithm/lgp/lgpEngine.h"

namespace Algorithm::LGP {
    /**
     * \brief Class in charge of executing a Program within its Environment.
     */
    class LGPExecutionEngine : public LGPEngine
    {
      protected:
        /// Default constructor is deleted.
        LGPExecutionEngine() = delete;

        /**
         * When true, all exceptions thrown when
         * fetching current instructions, operands are
         * caught and the current program Line is simply ignored.
         * When true, all lines of the Program are assumed to be
         * correct by construction, and any exception is re-thrown
         * for higher-level handling, thus stopping the program.
         * Exception thrown by getCurrentLine are never ignored.
         */
        bool ignoreException = false;

      public:
        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment given as a parameter.
         *
         * \param[in] env The Environment in which the Program will be executed.
         * \param[in] algorithmName name of the algorithm used.
         */
        LGPExecutionEngine(const Environment& env, std::string algorithmName) : LGPEngine(env, algorithmName){};

        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment given as a parameter instead of that of the
         * Program or its Environment.
         *
         * This constructor is useful for testing a Program on a different
         * Environment than its own.
         *
         * \param[in] executedAgent the const Program that will be executed or
         * generated.
         * \param[in] dataSrc The DataHandler with which the Program
         * will be executed.
         */
        template <class T>
        LGPExecutionEngine(
            std::shared_ptr<const LGPAgent> executedAgent,
            const std::vector<std::reference_wrapper<T>>& dataSrc)
            : LGPEngine(executedAgent, dataSrc){};

        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment of the given Program.
         *
         * \param[in] executedAgent the const Program that will be executed or
         * generated.
         */
        LGPExecutionEngine(std::shared_ptr<const LGPAgent> executedAgent)
            : LGPEngine(executedAgent){};

        /**
         * \brief Execute the current line of the program.
         *
         * \throws see fetchCurrentOperands, getCurrentInstruction.
         */
        void executeCurrentLine();

        /**
         * \brief Set the ignore exception property.
         *
         * \param[in] ignoreEx when true, all exceptions thrown when
         *            fetching current instructions, operands are
         *            caught and the current program Line is simply ignored.
         *            When true, all lines of the Program are assumed to be
         *            correct by construction, and any exception is re-thrown
         *            for higher-level handling, thus stopping the program.
         *            Exception thrown by getCurrentLine are never ignored.
         */
        void setIgnoreException(const bool ignoreEx) { this->ignoreException = ignoreEx; }

        /**
         * \brief Execute the program completely and returns the content of
         * register 0 to lgpAgent->nbOutputs.
         */
        virtual std::vector<double> execute() override;

        /// inherited from Program::ProgramEngine
        virtual void processLine() override;
    };
}; // namespace Algorithm::LGP
#endif // LGP_EXECUTION_ENGINE_H
