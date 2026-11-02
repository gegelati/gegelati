#ifndef EVOLUTION_ORIGIN_TPG_MODULE_H
#define EVOLUTION_ORIGIN_TPG_MODULE_H

#include "evolution/module.h"

#include "evolution/basicModule.h"

#include "representations/LGP.h"
#include "representations/TPG.h"

#include "dimensions/individualValueGenerator.h"
#include "dimensions/individualArchiveValueGenerator.h"

#include "mutation/nodeTransferMutation.h"
#include "metrics/outputMetric.h"

#include "evaluation/dataset.h"
#include "metrics/inputMetric.h"


namespace Evolution {

    /**
     * \brief Class for creating the original TPG framework
     * 
     * If used in parallelism, the determinism of the IDs of the members specifically is not guaranteed. (as members are dupplicated/mutated in parallel during TPG mutation)
     */
    class OriginTPGModule: public BasicModule {

        protected:

            /// \brief Representation of the members
            const Representations::Representation& memberRep;

            /// @brief TPG mutation using node transfer.
            std::shared_ptr<Mutation::Mutator> memberMutation;

            /// @brief Current member for tangled connection (need to be saved somewhere!)
            std::vector<std::shared_ptr<const Individual>> currentMemberForTangled;

            /// @brief archive for the member novelty.
            std::unique_ptr<Evaluation::DataSet> archive;

            /// \brief Maximum size of the archive
            size_t archiveSize;

            /// @brief Boolean indicating if the archive should be fixed (for classification for example).
            /// If it is fixed, it will not be updated after initialization
            bool isArchiveFixed;

            /**
             * \brief In initial implementation, an TPG individual becames "protected" when it is aggregated by another individual.
             * As long as the aggregation exist, the individual cannot be deleted by the selection pressure.
             */
            bool protectAggregated;

            /// @brief Vector of protected individuals
            std::map<std::shared_ptr<const Individual>, std::vector<uint64_t>, SharedLess<Individual>> protectedIndividuals;

        public:

            /**
             * \brief Default constructor of module
             */
            explicit OriginTPGModule(
                const Representations::TPG& TPGRep,
                const Representations::Representation& memberRep, 
                Evaluation::Problem& problem,
                size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, size_t nbValidationIterations=0, uint64_t seed = 0, size_t nbThreads = 1,
                size_t archiveSize = 50, bool isArchiveFixed = false, bool protectAggregated = true,
                std::shared_ptr<FitnessAssignment::Assigner> assigner = nullptr,
                std::shared_ptr<Selection::Selector> parentSelection = nullptr,
                std::shared_ptr<Selection::Selector> survivingSelection = nullptr,
                std::shared_ptr<Reproduction::Reproducer> breeder = nullptr,
                std::shared_ptr<Mutation::NodeTransferMutation> mutation = nullptr,
                std::shared_ptr<Mutation::Mutator> memberMutation = nullptr
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
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> getAllVertices() const;

            /**
             * \brief Get all the vertices in the current population
             */
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> getSubVerticesOf(const Individual& indiv) const;

            /**
             * \brief compute the answer of the current member in the population of the archive.
             */
            Data::DataValue computeMemberAnswer(
                const std::vector<Data::DataValue>& inputs, std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members);

            /**
             * \brief Get all the members contained in the proposed individuals
             */
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> getMembersOf(std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> individuals) const;

            /**
             * \brief Get all the members contained in the current population
             */
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> getAllMembers() const;

            /**
             * \brief Update the inputs and outputs stored inside the archive.
             */
            void updateArchive();
            /**
             * \brief Update the outputs stored inside the archive.
             */
            void updateArchiveOutputs();


            /**
             * \brief Execute basicModule method and set the archive.
             */
            virtual void initialize() override;

            /**
             * \brief Explore the population before and after generation.
             * - A check that individual in the protectedIndividual map aggregators still exists, if they don't, the individual is added back to the population.
             * - New offspring are controlled: if those offspring contains tangled members, they are set as aggregators.
             */
            virtual void updateProtectedIndividuals(std::vector<std::shared_ptr<const Individual>> populationBeforeGen);

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