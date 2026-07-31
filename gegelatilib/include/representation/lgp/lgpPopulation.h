
#ifndef LGP_AGENT_POPULATION_H
#define LGP_AGENT_POPULATION_H

#include "representation/population.h"
#include "representation/lgp/lgpIndividual.h"
#include "representation/lgp/lgpExecutionEngine.h"


namespace Representation {

    namespace LGP{

        /**
         * \brief Class representing a LGPPopulation used by the LGPRepresentation.
         * 
         * The Population is in charge of storing, creating, copying or removing Agents.
         * Basically, the population is the interface between the Representation and the Graph.
         */
        class LGPPopulation : public Population
        {
        protected:
            
            /**
             * \brief string used to spot the end of a line in the agent
             * description.
             */
            static const std::string lineSeparator;

            /// Environment for executing LGP 
            const LGPEnvironment& env;


            /**
             * \brief Get the LgpIndividual from a const Individual pointer.
             * 
             * \param[in] agent the Individual to cast.
             */
            virtual LgpIndividual& getLgpIndividualFromCst(const Individual& agent);
        public:

            /**
             * \brief LGPPopulation constructor.
             * 
             * \param[in] env the Environment for executing LGP
             * \param[in] outputs outputs of the agents.
             * \param[in] representationID id of the representation used.
             */
            LGPPopulation(const LGPEnvironment& env, const Output::OutputHandler& outputs, uint64_t representationID): Population(outputs, representationID), env(env){};

            /**
             * \brief Create a new LgpIndividual.
             * 
             * \param[in] graph the Graph associated with the Individual.
             * 
             * \return a shared pointer to the created Individual.
             */
            virtual const Individual& createAgent(EvoGraph::Graph& graph) override;

            /**
             * \brief Copy a LgpIndividual.
             * 
             * \param[in] agent the Individual to copy.
             * \param[in] graph the Graph associated with the Individual.
             * 
             * \return a shared pointer to the created Individual.
             */
            virtual const Individual& copyAgent(const Individual& agent, EvoGraph::Graph& graph) override;

            /**
             * \brief Empty a LgpIndividual of its program.
             * 
             * \param[in] agent the Individual to empty.
             * \param[in] graph the Graph associated with the Individual.
             * 
             * \return a shared pointer to the created Individual.
             */
            virtual void emptyAgent(const Individual& agent, EvoGraph::Graph& graph) override;

            /**
             * \brief Set the constant at the given index for the given agent.
             * 
             * \param[in] agent the Individual to modify.
             * \param[in] index the index of the constant to set.
             * \param[in] value the value to set.
             */
            virtual void setConstantAt(const Individual& agent, size_t index, const Data::Constant& value);

            /**
             * \brief Delete a line to the given agent at the given index.
             * 
             * \param[in] agent the Individual to modify.
             * \param[in] index the index at which the new line should be deleted.
             */
            virtual void removeLine(const Individual& agent, size_t index);

            /**
             * \brief Add a new line to the given agent at the given index.
             * 
             * \param[in] agent the Individual to modify.
             * \param[in] index the index at which the new line should be added.
             * 
             * \return a reference to the newly added LGPLine.
             */
            virtual const LGPLine& addNewLine(const Individual& agent, size_t index);
            
            /**
             * \brief Add a new line to the given agent at the end of the agent.
             * 
             * \param[in] agent the Individual to modify.
             * 
             * \return a reference to the newly added LGPLine.
             */
            virtual const LGPLine& addNewLine(const Individual& agent);

            /**
             * \brief Add a copy of the given line to the given agent at the end of the agent.
             * 
             * \param[in] agent the Individual to modify.
             * \param[in] newLine the line to copy.
             * \param[in] index the index at which the new line should be added.
             */
            virtual void addNewLine(const Individual& agent, const LGPLine& newLine, size_t index);

            /**
             * \brief Add a copy of the given line to the given agent at the end of the agent.
             * 
             * \param[in] agent the Individual to modify.
             * \param[in] newLine the line to copy.
             */
            virtual void addNewLine(const Individual& agent, const LGPLine& newLine);

            /**
             * \brief Swap two lines of the given agent.
             * 
             * \param[in] agent the Individual to modify.
             * \param[in] index1 the index of the first line to swap.
             * \param[in] index2 the index of the second line to swap.
             */
            virtual void swapLines(const Individual& agent, size_t index1, size_t index2);

            /**
             * \brief Get a line of the given agent at the given index.
             * 
             * \param[in] agent the Individual to access.
             * \param[in] index the index of the line to get.
             * 
             * \return a reference to the LGPLine at the given index.
             */
            virtual const LGPLine& getLine(const Individual& agent, size_t index) const;

            /**
             * \brief Get a line of the given agent at the given index for mutation.
             * 
             * \param[in] agent the Individual to access.
             * \param[in] index the index of the line to get.
             * 
             * \return a reference to the LGPLine at the given index.
             */
            virtual LGPLine& getLineForMutation(const Individual& agent, size_t index);

            /**
             * \brief Identify the introns of the given agent.
             * 
             * \param[in] agent the Individual to analyze.
             */
            virtual uint64_t identifyIntrons(const Individual& agent);

            /**
             * \brief Clear the introns of all the agents
             */
            virtual void clearAgentsIntrons();        

            /**
             * \brief Clear the introns of the agent
             * 
             * \param[in] agent the Individual to clear.
             */
            virtual void clearAgentIntrons(const Individual& agent);
            
            /**
             * \brief Check if two LGP have the same behavior.
             *
             * Two LGP have the same behaviour if their sequence of non-intron
             * Lines are strictly identical (i.e. same instructions and operands, in
             * the same order, and used Constant with identical values).
             *
             * \param[in] agent1 first lgp.
             * \param[in] agent2 second lgp.
             */
            bool hasIdenticalBehavior(const Individual& agent1, const Individual& agent2) const;

            /**
             * \brief Create a new ExecutionEngine for this Representation.
             */
            virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;

            


            /**
             * \brief Set a specific output index to an agent.
             * 
             * 
             * \param[in] agent the Individual to empty.
             * \param[in] newOutputIndex new output index set
             * \param[in] location location of input at which the new index is set
             */
            void setOutputIndex(const LgpIndividual& agent, size_t newOutputIndex, size_t location);

            /**
             * \brief Reads the content of the operands and puts it in the line
             * passed in parameter
             *
             * \param[in] str the string to parse
             * \param[in] line the line to fill with the parsed informations
             */
            virtual void readOperands(std::string& str, LGPLine& line);

            /**
             * \brief Reads the content of a line
             */
            virtual void readLines(std::string instructionsStr, const Individual& agent);

        };
    } // namespace LGP
}; // namespace Representation

#endif // LGP_AGENT_POPULATION_H
