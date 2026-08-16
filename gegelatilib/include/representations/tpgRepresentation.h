
#ifndef TPG_REPRESENTATION_H
#define TPG_REPRESENTATION_H

#include <memory>
#include <vector>

#include "evolution/representation.h"
#include "evolution/population.h"

namespace Representations {

    /**
     * \brief Abstract class representing a TPGRepresentation
     */
    class TPGRepresentation : public Evolution::Representation
    {
        protected:

            /// @brief Representation of the context members
            const Evolution::Representation& contextMemberRep;

            /// @brief Context Member population
            const Evolution::Population& contextMemberPop;

            /// @brief Tangled Population
            const Evolution::Population& tangledPopulation;


        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Evolution::Representation> cloneUniquePtr() const;

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] contextMemberRep Representation of the context members
             * \param[in] contextMemberPop Context Member population
             * \param[in] tangledPopulation Tangled Population
             * \param[in] nbNodesMin the minimum number of nodes in the representation.
             * \param[in] nbNodesMax the maximum number of nodes in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            TPGRepresentation(const Evolution::Representation& contextMemberRep, const Evolution::Population& contextMemberPop,  const Evolution::Population& tangledPopulation, size_t nbNodesMin, size_t nbNodesMax=0, std::string representationName = "TPG", std::string representationColor = "#922DB4")
                : Evolution::Representation(nbNodesMin, nbNodesMax, representationName, representationColor), contextMemberRep{contextMemberRep}, contextMemberPop{contextMemberPop}, tangledPopulation{tangledPopulation} {};
        

        /**
         * \brief set the number of input sources. 
         * 
         * \param[in] inputSources get the dimensions of the input sources. The inputSources is not copied to allow dupplication of sources, for example with parallelism.
         */
        virtual void setInputDimensions(const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) override;

        /**
         * \brief return the genotype template an individual.
         */
        virtual std::unique_ptr<const Node::GenotypeTemplate> getGenotypeTemplate() const;

        /**
         * \brief individual nodes should have two values, one individual and one integer.
         * 
         * \param[in] indiv Individual controlled.
         */
        virtual bool isValid(const Evolution::Individual& indiv) const override;

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
        virtual std::vector<double> executeIndividual(
            const Evolution::Individual& indiv, const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) const override;

        };
}; // namespace LGP_Representation


#endif
