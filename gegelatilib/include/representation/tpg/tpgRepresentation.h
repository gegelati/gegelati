
#ifndef TPG_REPRESENTATION_H
#define TPG_REPRESENTATION_H

#include <memory>
#include <vector>

#include "representation/tpg/archive.h"
#include "representation/representation.h"
#include "representation/tpg/tpgPolicyStats.h"
#include "representation/tpg/tpgPopulation.h"
#include "representation/tpg/tpgMutator.h"
#include "representation/tpg/tpgIndividual.h"
#include "representation/tpg/tpgJob.h"

#include "learn/learningEnvironment.h"

namespace Representation::TPG {

    /**
     * \brief Abstract class representing a TPGRepresentation
     */
    class TPGRepresentation : public Representation
    {
        protected:

            /// ID of the program representation associated with the TPG individuals.
            uint64_t programRepresentationID;

            /// TPGArchive used during the training process
            std::unique_ptr<TPGArchive> archive;

        public:

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] programRepresentation the sub-representation used to manipulate programs.
             * \param[in] parameters the LearningParameters used by the Representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            TPGRepresentation(const Representation& programRepresentation, std::unique_ptr<RepresentationParameters> parameters = std::make_unique<RepresentationParameters>(), std::string representationName = "TPG", std::string representationColor = "#A0FF33")
                : Representation(std::move(parameters), representationName, representationColor) {
                archive = std::make_unique<TPGArchive>(this->params->tpg.archiveSize, this->params->tpg.archivingProbability);
                this->setProgramRepresentation(programRepresentation);
            };

            /**
             * \brief Add the program sub-representation to the TPGRepresentation.
             * 
             */
            void setProgramRepresentation(const Representation& programRepresentation);


            /**
             * \brief Get the TPGArchive used by the LGPRepresentation.
             */
            const TPGArchive& getArchive() const;
            
            /**
             * \brief Clear all the parts of individuals that are not used, such as introns for LGPs
             */
            virtual void clearUnusedIndividualParts() override {};


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
             * \brief Initialize the sub-representations of the representation
             * 
             * \param[in] rng deterministic random generator
             * \param[in] outputs outputs needed for the representation.
             * \param[in] dataSource input sources of the representation.
             * \param[in] graph the EvoGraph::Graph used by the representation.
             */
            virtual void initSubRepresentations(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * Copy and return a uniqure pointer of the representation
             */
            virtual std::unique_ptr<Representation> copy() const override;

            /**
             * \brief Takes a given Individual and creates a job containing it.
             *
             * \param[in] individual the Individual to be evaluated.
             * \param[in] mode the mode of the training, determining for example
             * if we generate values that we only need for training.
             * \param[in] rng deterministic random generator
             * \param[in] idx The index of the job, can be used to organize a map
             * for example.
             *
             * \return A job representing the individual.
             */
            virtual std::shared_ptr<Job> createJob(const Individual& individual, Learn::LearningMode mode, RNG::RNG& rng, int idx = 0) const override;

            /**
             * \brief Inherited method to create the policy stats of the representation
             * 
             * This policy stats contains a program sub policy stats
             */
            virtual std::shared_ptr<PolicyStats> createPolicyStats() const override;
            
            /**
             * \brief Inherited method to update the representation after evaluation of a set of jobs.
             * 
             * Does the merging of the archives after evaluation.
             */
            virtual void updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode) override;

            /**
             * \brief Inherited method to clear all the unused sub individuals
             * 
             * The sub individuals are the individuals used by the program sub-representation.
             */
            virtual std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>> getUsedSubIndividuals() const override;

            /** 
             * \brief Inherited method to print a TpgIndividual.
             * 
             * The TPG individual prints the vertex it points to.
             */
            void printIndividual(const Individual& individual, FILE* pFile, std::string offset, std::set<uint64_t>& printedIndividualID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const override;

            /**
             * \brief Inherited method to read a TpgIndividual.
             */
            virtual const Individual& readIndividual(std::smatch& matches) override;

            /**
             * \brief inherrit from representation class
             */
            virtual void printCodeGenIndividuals(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Individual>>& individuals, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>& subIndividuals) const;

            /**
             * \brief Link an individual to a corresponding vertex
             * 
             * \param[in] individual the individual linked to the vertex.
             * \param[in] vertex the vertex linked to the individual.
             */
            virtual void linkIndividualVertex(const Individual& individual, const EvoGraph::Vertex& vertex) override;
            
    };
}; // namespace TPG_Representation

namespace Representation{
    /**
     * To make the aglrotihm accessible from representation namespace
     */
    using TPGRepresentation = TPG::TPGRepresentation;
}

#endif
