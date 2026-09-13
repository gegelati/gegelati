
#ifndef TPG_REPRESENTATION_H
#define TPG_REPRESENTATION_H

#include <memory>
#include <string>
#include <vector>

#include "evolution/representation.h"
#include "evolution/population.h"
#include "dimensions/numericRange.h"

namespace Representations {

    /**
     * \brief Abstract class representing a TPGRepresentation
     */
    class TPGRepresentation : public Evolution::Representation
    {
        protected:

            /// @brief The number of actions in the TPGRepresentation.
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
                const Evolution::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const override;

            /// @brief Unique pointer of the genotypeTemplate, as it is fixed during evolution
            std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate;

            /// @brief Member available for the TPG individuals
            std::vector<std::shared_ptr<const Evolution::Individual>> availableMembers;

            /// \brief Individual available for tangled connexions
            std::vector<std::shared_ptr<const Evolution::Individual>> availableForTangled;
            
            /**
             * \brief Create the genotype template grammar
             */
            virtual void setGenotypeTemplate();
    
        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Evolution::Representation> cloneUniquePtr() const override;

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] inputDimensions the dimensions of the input sources.
             * \param[in] nbActions The number of actions in the TPGRepresentation.
             * \param[in] nbNodesMin the minimum number of nodes in the representation.
             * \param[in] nbNodesMax the maximum number of nodes in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            TPGRepresentation(std::vector<Dimensions::Requirement> inputDimensions, size_t nbActions, size_t nbNodesMin, size_t nbNodesMax=0, std::string representationName = "TPG", std::string representationColor = "#922DB4")
                : Evolution::Representation(
                    inputDimensions, Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::between(0, nbActions - 1)), nbNodesMin, nbNodesMax, representationName, representationColor), 
                    nbActions{nbActions} {
                this->setGenotypeTemplate();
            };

            /**
             * \brief return the genotype template an LGP individual, defined in setGenotypeTemplate.
             */
            virtual std::unique_ptr<Node::GenotypeTemplate> getGenotypeTemplate() const override;


            /**
             * Set the current available members
             */
            virtual void setAvailableMembers(const std::vector<std::shared_ptr<const Evolution::Individual>>& members);

            /**
             * Set the current available individuals for tangled connection.
             * 
             * The genotype template is updated if the size of the current list was empty, or if the new one is. (to specify a use of multiGenerator)
             */
            virtual void setAvailableTangledIndiv(const std::vector<std::shared_ptr<const Evolution::Individual>>& tangledIndiv);

        };
}; // namespace LGP_Representation


#endif
