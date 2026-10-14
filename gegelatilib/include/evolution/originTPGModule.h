#ifndef EVOLUTION_ORIGIN_TPG_MODULE_H
#define EVOLUTION_ORIGIN_TPG_MODULE_H

#include "evolution/module.h"

#include "evolution/basicModule.h"

#include "representations/LGP.h"
#include "representations/TPG.h"

#include "dimensions/individualValueGenerator.h"
#include "dimensions/individualArchiveValueGenerator.h"

#include "evaluation/dataset.h"
#include "metrics/inputMetric.h"


namespace Evolution {

    /**
     * \brief Class for creating the original TPG framework
     */
    class OriginTPGModule: public BasicModule {

        protected:

            /// \brief Representation of the members
            const Representations::Representation& memberRep;

            /// @brief Current member for tangled connection (need to be saved somewhere!)
            std::vector<std::shared_ptr<const Individual>> currentMemberForTangled;

            /// @brief archive for the member novelty.
            std::unique_ptr<Evaluation::DataSet> archive;

        public:

            /**
             * \brief Default constructor of module
             */
            explicit OriginTPGModule(
                const Representations::TPG& TPGRep,
                const Representations::Representation& memberRep, 
                Evaluation::Problem& problem,
                size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, uint64_t seed = 0, size_t nbThreads = 1,
                bool useArchive = true,
                std::shared_ptr<FitnessAssignment::Assigner> assigner = nullptr,
                std::shared_ptr<Selection::Selector> parentSelection = nullptr,
                std::shared_ptr<Selection::Selector> survivingSelection = nullptr,
                std::shared_ptr<Reproduction::Reproducer> breeder = nullptr,
                std::shared_ptr<Mutation::Mutator> mutation = nullptr
            );

            /**
             * \brief Method used during initialize to create the column header of the basicLoggers
             * 
             * Add number of vertex and member currently used
             */
            virtual void setBasicLoggerHeader() override;

            /**
             * \brief Get all the vertices in the current population
             */
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> getAllVertices();

            /**
             * \brief compute the answer of the current member in the population of the archive.
             */
            Data::DataValue computeMemberAnswer(
                const std::vector<Data::DataValue>& inputs, std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members);

            /**
             * \brief Get all the members contained in the proposed individuals
             */
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> getMembersOf(std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> individuals);

            /**
             * \brief Get all the members contained in the current population
             */
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> getAllMembers();

            /**
             * \brief Update the inputs stored inside the archive.
             */
            void updateArchiveInputs();

            /**
             * \brief Override to update the generator before launching basicModule::doGeneration
             */
            virtual void doGeneration(size_t generationNumber) override;

            /**
             * \brief Method used during doGeneration to add a raw to the basicLoggers
             * 
             * Add number of vertex and member currently used
             * 
             * \param[in] generationNumber the index of the generation
             */
            virtual void setNewBasicLoggerRow(size_t generationNumber) override;
    };
};

#endif // EVOLUTION_BASIC_MODULE_H