
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

            /// @brief Representation of the context members
            const Evolution::Representation& contextMemberRep;

            /// @brief Context Member population
            const Evolution::Population& contextMemberPop;


            /**
             * \brief execute each node as a bidding component. 
             * 
             * A node is of shape: {a, b}.
             *  - "a" is a context member individual.
             *  - "b" is the destination action index.
             * 
             * \param[in] indiv Individual executed
             * \param[in] inputSources input sources on which the individual is executed.
             */
            virtual Data::DataValue executeIndividualRaw(
                const Evolution::Individual& indiv, const std::vector<Data::DataView>& inputSources) const override;
    
            /**
             * \brief Create the genotype requirements grammar
             */
            virtual void setGenotypeRequirements() override;
        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Evolution::Representation> cloneUniquePtr() const override;

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] inputDimensions the dimensions of the input sources.
             * \param[in] nbActions The number of actions in the TPGRepresentation.
             * \param[in] contextMemberRep Representation of the context members
             * \param[in] contextMemberPop Context Member population
             * \param[in] nbNodesMin the minimum number of nodes in the representation.
             * \param[in] nbNodesMax the maximum number of nodes in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            TPGRepresentation(std::vector<Dimensions::Requirement> inputDimensions, size_t nbActions, const Evolution::Representation& contextMemberRep, const Evolution::Population& contextMemberPop, size_t nbNodesMin, size_t nbNodesMax=0, std::string representationName = "TPG", std::string representationColor = "#922DB4")
                : Evolution::Representation(
                    inputDimensions, Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::between(0, nbActions - 1)), nbNodesMin, nbNodesMax, representationName, representationColor), 
                    nbActions{nbActions}, contextMemberRep{contextMemberRep}, contextMemberPop{contextMemberPop} {
                    this->setTangled(true);

                    if(!Dimensions::DimensionFlow::acceptsRequirements(inputDimensions, contextMemberRep.getDimensionFlow().getInputDimensions())) {
                        throw std::runtime_error("TPGRepresentation:Constructor: Input Dimensions set is not compatible with the context member representation input dimensions");
                    }
                    if(contextMemberRep.getDimensionFlow().getOutputDimension().isCompatibleWith(Dimensions::Requirement::scalar<double>(Dimensions::NumericRange<double>::unbounded())) == false) {
                        throw std::runtime_error("TPGRepresentation:Constructor: Context member representation output dimension is not compatible with the required scalar<double> output dimension");
                    }
                };

        /**
         * \brief individual nodes should have two values, one individual and one integer or individual.
         * 
         * The first individual must be valid regarding the member representation, the second (if there is) must be valid regarding the tangled representation.
         * Additionnally, this second must not be the same individual as the tested one.
         * Important: This method do not check for cycles of tangled individuals.
         * 
         * \param[in] indiv Individual controlled.
         */
        virtual bool isValid(const Evolution::Individual& indiv) const override;


        };
}; // namespace LGP_Representation


#endif
