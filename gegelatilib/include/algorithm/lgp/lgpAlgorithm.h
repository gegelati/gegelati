
#ifndef LGP_ALGORITHM_H
#define LGP_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/algorithm.h"
#include "algorithm/lgp/lgpManager.h"
#include "algorithm/lgp/lgpMutator.h"
#include "algorithm/lgp/lgpAgent.h"
#include "algorithm/lgp/lgpPolicyStats.h"
#include "algorithm/lgp/lgpCodeGenerationEngine.h"
namespace Algorithm::LGP {

    /**
     * \brief Abstract class representing a LGPAlgorithm
     */
    class   LGPAlgorithm : public Algorithm
    {
        protected:

            /// Environment for executing LGP 
            std::unique_ptr<Environment> env;

            /// Instruction Set used by the LGPAlgorithm
            const Instructions::Set& iSet;

            /// @brief regex use to read an lgpAgent from a .dot file.
            static const std::string lgpAgentRegex;

        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] algorithmColor name of the algorithm used.
             */
            LGPAlgorithm(const Learn::LearningParameters& params, const Instructions::Set& iSet, std::string algorithmName = "LGP", std::string algorithmColor = "#922DB4")
                : Algorithm(params, algorithmName, algorithmColor), iSet{iSet} {};


            /**
             * \brief Get the Environment used by the LGPAlgorithm.
             */
            const Environment& getEnvironment() const;


            
            /**
             * \brief Clear all the parts of agents that are not used, such as introns for LGPs
             */
            virtual void clearUnusedAgentParts() override {};


            
            /**
             * \brief Initialize the managerof the algorithm
             * 
             * \param[in] outputs outputs needed for the algorithm.
             */
            virtual void initManager() override;

            /**
             * \brief Initialize the mutator of the algorithm
             */
            virtual void initMutator() override;

            /**
             * \brief Initialize the algorithm.
             */
            virtual void initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Inherited method to create the policy stats of the algorithm
             */
            virtual std::shared_ptr<PolicyStats> createPolicyStats() const override;

            /**
             * Copy and return a uniqure pointer of the algorithm
             */
            virtual std::unique_ptr<Algorithm> copy() const override;

            /** 
             * \brief Inherited method to print a LGPagent.
             * 
             * The LGP agent prints the different lines of its program.
             */
            void printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const override;
            

            /**
             * \brief Inherited method to read a LGPAgent.
             * 
             * This method will use the current line to get the constant of the agent.
             * Then it will get the next line to read the instruction
             */
            virtual const Agent& readAgent(std::smatch& matches) override;

            /**
             * \brief inherrit from algorithm class
             */
            virtual void printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const;
        };
}; // namespace LGP_Algorithm


namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using LGPAlgorithm = LGP::LGPAlgorithm;
}

#endif
