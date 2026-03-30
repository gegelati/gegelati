/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2021)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef LGP_ENGINE_H
#define LGP_ENGINE_H

#include <memory>

#include "data/primitiveTypeArray.h"
#include "data/untypedSharedPtr.h"
#include "algorithm/lgp/lgpAgent.h"
#include "algorithm/executionEngine.h"

namespace Algorithm::LGP {
    /**
     * \brief This abstract class is the base class for any program engine
     * (generation and execution)
     *
     * This class holds the common algortithms and members required to generate
     * or execute a program for a given Environment.
     */
    class LGPEngine : public ExecutionEngine
    {
      protected:

        /// Default constructor is deleted.
        LGPEngine() = delete;

        /// Registers used for the Program execution.
        Data::PrimitiveTypeArray<double>
            registers; // If the type of registers attribute is
                       // changed one day
        // make sure to update the Program::identifyIntrons()
        // method as it create its own
        // Data::PrimitiveTypeArray<double> to keep track of
        // accessed addresses.

        /// Data sources (including registers) used in the Program.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dataScsConstsAndRegs;

        /// Program counter of the execution engine.
        uint64_t programCounter;

        /// casted executed agent
        std::optional<std::reference_wrapper<const LGPAgent>> lgpExecutedAgent;

      protected:
        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment given as a parameter.
         *
         * \param[in] env The Environment in which the Program will be executed.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] algorithmID id of the algorithm used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        LGPEngine(const LGPEnvironment& env, const Output::OutputHandler& outputs, uint64_t algorithmID, bool isTraining = false)
            : ExecutionEngine{outputs, algorithmID, isTraining}, programCounter{0}, registers{env.getNbRegisters()}
        {
            dataSources = env.getDataSources();
            // Setup the data sources
            dataScsConstsAndRegs.push_back(this->registers);

            if (env.getNbConstants() > 0) {
                dataScsConstsAndRegs.push_back(env.getFakeDataSources().at(1));
            }

            // Cannot use insert here because it dataSourcesAndRegisters
            // requires constnessand dataSrc data are not const...
            for (auto data : env.getDataSources()) {
                
                dataScsConstsAndRegs.push_back(data.get());
            }
            
        }

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
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        template <class T>
        LGPEngine(const LGPAgent& executedAgent, 
                      const std::vector<std::reference_wrapper<T>>& dataSrc, bool isTraining = false)
            : ExecutionEngine{executedAgent, executedAgent.getOutputs(), isTraining}, programCounter{0},
              registers{executedAgent.getEnvironment().getNbRegisters()}
        {
            // Check that T is either convertible to a const DataHandler
            static_assert(
                std::is_convertible<T&, const Data::DataHandler&>::value);
            // Setup the data sources
            this->dataScsConstsAndRegs.push_back(this->registers);

            if (executedAgent.getEnvironment().getNbConstants() > 0) {
                this->dataScsConstsAndRegs.push_back(
                    executedAgent.cGetConstantHandler());
            }

            // Cannot use insert here because it dataSourcesAndRegisters
            // requires constnessand dataSrc data are not const...
            for (std::reference_wrapper<T> data : dataSrc) {
                this->dataScsConstsAndRegs.push_back(data.get());
                this->dataSources.push_back(data.get());
            }

            // Set the executedAgent
            this->setExecutedAgent(executedAgent);
        };

        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment of the given Program.
         *
         * \param[in] executedAgent the const Program that will be executed or
         * generated.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        LGPEngine(const LGPAgent& executedAgent,  bool isTraining = false)
            : LGPEngine(executedAgent, executedAgent.getEnvironment().getDataSources(), isTraining){};

        /**
         * \brief operator parenthesis used when iterating through the program
         * with the function iterationThroughtProgram
         */
        virtual void processLine() = 0;

      public:

        /**
         * \brief Method for changing the agent executed by a
         * LGPEngine.
         *
         * \param[in] newExecutedAgent the LGPAgent executed by the lgpEngine
         */
        virtual void setExecutedAgent(const Agent& newExecutedAgent) override;

        /**
         * \brief Method for changing the dataSources on which the Program will
         * be executed.
         *
         * \param[in] dataSrc The vector of DataHandler references with which
         * the Program will be executed.
         * \throws std::runtime_error if the Environment references by the
         * Program is incompatible with the given dataSources.
         */
        virtual void setDataSources(
            const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSrc) override;

        /**
         * \brief Inherrited from ExecutionEngine
         */
        virtual const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        getDataSources() const override;

        /**
         * \brief Increments the programCounter and checks for the end of the
         * Program.
         *
         * This method will automatically skip intron lines of the Program when
         * searching for the next Line to execute.
         *
         * \return true if the Program of the ProgramExecutionEngine has a Line
         * for the new programCounter value, and false otherwise.
         */
        const bool next();

        /**
         * \brief Get the LGPLine corresponding to the current
         * programCounter.
         *
         * \return a const ref to the Line from the Program indexed by the
         * current programCounter.
         * \throw std::out_of_range if the programCounter exceeds the number of
         * lines of the program.
         */
        const LGPLine& getCurrentLine() const;

        /**
         * \brief Get the Instruction corresponding to the current
         * programCounter.
         *
         * \return the Instruction from the Environment Instruction::Set for
         * the Line of the Program indexed by the current programCounter.
         * \throw std::out_of_range if the programCounter exceeds the number of
         * lines of the program or if the instruction index contained in the
         * current Line exceeds the number of Instruction in the Environment
         * Instructions::Set.
         */
        const Instructions::Instruction& getCurrentInstruction() const;

        /**
         * \brief Get the operands for the current Instruction.
         *
         * This method fetches from the dataSourcesAndRegisters the operands
         * indexed in the current Line of the Program. To get the correct data,
         * the method Uses the data types of the current Instruction of the
         * program.
         *
         * \param[in,out] operands std::vector where the fetched operands will
         * be inserted. \throws std::invalid_argument if the data type of the
         * current Instruction is not provided by the indexed DataHandler.
         * \throws std::out_of_range if the given address is invalid for the
         * indexed DataHandler, with the given data type, or if the indexed
         *         DataHandler does not exist.
         */
        const void fetchCurrentOperands(
            std::vector<Data::UntypedSharedPtr>& operands) const;

        /**
         * \brief Get the location for the current Instruction.
         *
         * This method fetches from the dataSourcesAndRegisters the operands
         * indexed in the current Line of the Program. To get the correct data,
         * the method Uses the data types of the current Instruction of the
         * program.
         *
         * \param[in] idxOp std::vector where the fetched operands will
         * be inserted. \throws std::invalid_argument if the data type of the
         * current Instruction is not provided by the indexed DataHandler.
         * \throws std::out_of_range if the given address is invalid for the
         * indexed DataHandler, with the given data type, or if the indexed
         *         DataHandler does not exist.
         */
        uint64_t getOperandLocation(uint64_t idxOp) const;

        /**
         * \brief Function that iterates through the lines of the program and
         * execute the function processLine().
         *
         * For each line that is not an intron, this function calls
         * processLine(). This function can be overloaded for example to execute
         * or to generate the non introns lines.
         */
        virtual void iterateThroughtProgram(const bool ignoreException);

        /**
         * \brief Return the current registers value of the program indicated.
         *
         * \param nbRegisters The number of register values that are taken
         * (starting from the register 0)
         *
         * \return a vector containing the different value of the registers.
         */
        virtual std::vector<double> getRegisterValues(uint64_t nbRegisters);
    };


} // namespace Algorithm::LGP

#endif // LGP_ENGINE_H
