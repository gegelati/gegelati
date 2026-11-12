
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
     * \brief Tangled Program Graph representation: the nodes of a vertex bid with their member, the best one gives its destination.
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
             *  - "b" is the destination: an action index, or another vertex.
             * 
             * \param[in] genotype genotype executed
             * \param[in] inputSources input sources on which the individual is executed.
             */
            virtual Data::DataValue executeGenotype(
                const GraphBased::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const override;


            /// \brief the minimum number of members in the representation.
            size_t nbMembersMin;
            /// \brief the maximum number of members in the representation.
            size_t nbMembersMax;
            
            /**
             * \brief Create the genotype constraint grammar
             */
            virtual void setGenotypeConstraint() override;

    
        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Representation> cloneOnlyRepresentation() const override;

            /// Inherited from Named, adds the number of actions and of members.
            virtual std::string getDetails() const override
            {
                return this->getName() + "(" + std::to_string(this->nbActions) + " actions, " + std::to_string(this->nbMembersMin) + " to "
                       + std::to_string(this->nbMembersMax) + " members)";
            }

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] inputDimensions the dimensions of the input sources.
             * \param[in] nbActions The number of actions in the TPG.
             * \param[in] nbMembersMin the minimum number of members in the representation.
             * \param[in] nbMembersMax the maximum number of members in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor color of the representation in the dot files.
             */
            TPG(std::vector<Dimensions::Requirement> inputDimensions, size_t nbActions, size_t nbMembersMin, size_t nbMembersMax=0, std::string representationName = "TPG", std::string representationColor = "#2db436")
                : Representation(
                    inputDimensions, Dimensions::Requirement(Data::DataType::type<size_t>(), Dimensions::NumericRange<size_t>::between(0, nbActions - 1)), representationName, representationColor), 
                    nbActions{nbActions}, nbMembersMin{nbMembersMin}, nbMembersMax{nbMembersMax} {
                        if(nbMembersMax == 0) {
                            this->nbMembersMax = nbMembersMin;
                        }
                        this->setGenotypeConstraint();
                    };


            /**
             * \brief return the number of action accessible by the tpg rep.
             */
            const size_t& getNbActions() const;


        };
}; // namespace Representations


#endif // REPRESENTATIONS_TPG_H
