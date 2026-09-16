
#ifndef REPRESENTATIONS_TPG_H
#define REPRESENTATIONS_TPG_H

#include <memory>
#include <string>
#include <vector>

#include "representations/representation.h"
#include "individual.h"
#include "dimensions/numericRange.h"

 namespace Representations {

    /**
     * \brief Abstract class representing a TPG
     */
    class TPG : public Representation
    {
        protected:

            /// @brief The number of actions in the TPG.
            const size_t nbActions;


            /**
             * \brief execute each node as a bidding component. 
             * 
             * A node is of shape: {a, b}.
             *  - "a" is a context member individual.
             *  - "b" is the destination action index.
             * 
             * \param[in] genotype genotype executed
             * \param[in] inputSources input sources on which the individual is executed.
             */
            virtual Data::DataValue executeGenotype(
                const GraphBased::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const override;

            /// @brief Unique pointer of the genotypeTemplate, as it is fixed during evolution
            std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator;

            /// @brief Member available for the TPG individuals
            std::vector<std::shared_ptr<const Individual>> availableMembers;

            /// \brief Individual available for tangled connexions
            std::vector<std::shared_ptr<const Individual>> availableForTangled;

            /// \brief the minimum number of members in the representation.
            size_t nbMembersMin;
            /// \brief the maximum number of members in the representation.
            size_t nbMembersMax;
            
            /**
             * \brief Create the genotype constraint grammar
             */
            virtual void setGenotypeConstraint() override;

            /**
             * \brief Create the genotype template grammar
             */
            virtual void setGenotypeGenerator();
    
        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Representation> cloneOnlyRepresentation() const override;

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] inputDimensions the dimensions of the input sources.
             * \param[in] nbActions The number of actions in the TPG.
             * \param[in] nbMembersMin the minimum number of members in the representation.
             * \param[in] nbMembersMax the maximum number of members in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            TPG(std::vector<Dimensions::Requirement> inputDimensions, size_t nbActions, size_t nbMembersMin, size_t nbMembersMax=0, std::string representationName = "TPG", std::string representationColor = "#922DB4")
                : Representation(
                    inputDimensions, Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::between(0, nbActions - 1)), representationName, representationColor), 
                    nbActions{nbActions}, nbMembersMin{nbMembersMin}, nbMembersMax{nbMembersMax} {
                        if(nbMembersMax == 0) {
                            this->nbMembersMax = nbMembersMin;
                        }
                        this->setGenotypeConstraint();
                        this->setGenotypeGenerator();
                    };

            /**
             * \brief return the genotype template an LGP individual, defined in setGenotypeTemplate.
             */
            virtual std::unique_ptr<GraphBased::GenotypeGenerator> getGenotypeGenerator() const override;


            /**
             * Set the current available members
             */
            virtual void setAvailableMembers(const std::vector<std::shared_ptr<const Individual>>& members);

            /**
             * Set the current available individuals for tangled connection.
             * 
             * The genotype template is updated if the size of the current list was empty, or if the new one is. (to specify a use of multiGenerator)
             */
            virtual void setAvailableTangledIndiv(const std::vector<std::shared_ptr<const Individual>>& tangledIndiv);

        };
}; // namespace LGP_Representation


#endif // REPRESENTATION_TPG_H
