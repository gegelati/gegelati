
#ifndef LGP_REPRESENTATION_H
#define LGP_REPRESENTATION_H

#include <memory>
#include <vector>

#include "representation/representation.h"
#include "representation/lgp/lgpPopulation.h"
#include "representation/lgp/lgpMutator.h"
#include "representation/lgp/lgpIndividual.h"
#include "representation/lgp/lgpPolicyStats.h"
#include "representation/lgp/lgpCodeGenerationEngine.h"
#include "representation/lgp/environment.h"
namespace Representation::LGP {

    /**
     * \brief Abstract class representing a LGPRepresentation
     */
    class   LGPRepresentation : public Representation
    {
        protected:

            /// Environment for executing LGP 
            std::unique_ptr<LGPEnvironment> env;

            /// Instruction Set used by the LGPRepresentation
            const Instructions::Set& iSet;

            /// @brief regex use to read an lgpIndividual from a .dot file.
            static const std::string lgpIndividualRegex;

        public:

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] iSet the Instruction Set used by the LGPRepresentation.
             * \param[in] parameters the LearningParameters used by the Representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            LGPRepresentation(const Instructions::Set& iSet, std::unique_ptr<RepresentationParameters> parameters = std::make_unique<RepresentationParameters>(), std::string representationName = "LGP", std::string representationColor = "#922DB4")
                : Representation(std::move(parameters), representationName, representationColor), iSet{iSet} {};


            /**
             * \brief Get the Environment used by the LGPRepresentation.
             */
            const LGPEnvironment& getEnvironment() const;


            
            /**
             * \brief Clear all the parts of individuals that are not used, such as introns for LGPs
             */
            virtual void clearUnusedIndividualParts() override;


            
            /**
             * \brief Initialize the populationof the representation
             * 
             * \param[in] outputs outputs needed for the representation.
             */
            virtual void initPopulation() override;

            /**
             * \brief Initialize the mutator of the representation
             */
            virtual void initMutator() override;

            /**
             * \brief Initialize the representation.
             */
            virtual void initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Inherited method to create the policy stats of the representation
             */
            virtual std::shared_ptr<PolicyStats> createPolicyStats() const override;

            /**
             * Copy and return a uniqure pointer of the representation
             */
            virtual std::unique_ptr<Representation> copy() const override;

            /** 
             * \brief Inherited method to print a LgpIndividual.
             * 
             * The LGP individual prints the different lines of its program.
             */
            void printIndividual(const Individual& individual, FILE* pFile, std::string offset, std::set<uint64_t>& printedIndividualID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const override;
            

            /**
             * \brief Inherited method to read a LgpIndividual.
             * 
             * This method will use the current line to get the constant of the individual.
             * Then it will get the next line to read the instruction
             */
            virtual const Individual& readIndividual(std::smatch& matches) override;

            /**
             * \brief inherrit from representation class
             */
            virtual void printCodeGenIndividuals(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Individual>>& individuals, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>& subIndividuals) const;
        };
}; // namespace LGP_Representation


namespace Representation{
    /**
     * To make the aglrotihm accessible from representation namespace
     */
    using LGPRepresentation = LGP::LGPRepresentation;
}

#endif
