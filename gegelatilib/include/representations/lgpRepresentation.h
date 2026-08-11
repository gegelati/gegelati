
#ifndef LGP_REPRESENTATION_H
#define LGP_REPRESENTATION_H

#include <memory>
#include <vector>

#include "evolution/representation.h"
#include "instructions/set.h"

namespace Representations {

    /**
     * \brief Abstract class representing a LGPRepresentation
     */
    class LGPRepresentation : public Evolution::Representation
    {
        protected:

            /// Instruction Set used by the LGPRepresentation
            const Instructions::Set& iSet;

            /// The number of registers used by the LGPs
            size_t nbRegisters;
            
            /// Registers used for the Program execution.
            Data::PrimitiveTypeArray<double> registers; 

        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Evolution::Representation> cloneUniquePtr() const;

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] iSet the Instruction Set used by the LGPRepresentation.
             * \param[in] nbRegisters the number of registers used by the LGPs
             * \param[in] nbNodesMin the minimum number of nodes in the representation.
             * \param[in] nbNodesMax the maximum number of nodes in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            LGPRepresentation(const Instructions::Set& iSet, size_t nbRegisters, size_t nbNodesMin, size_t nbNodesMax=0, std::string representationName = "LGP", std::string representationColor = "#922DB4")
                : Evolution::Representation(nbNodesMin, nbNodesMax, representationName, representationColor), iSet{iSet}, nbRegisters{nbRegisters}, registers{nbRegisters} {};
        

        /**
         * \brief set the number of input sources. 
         * LGP representation adds register dataHandler as input.
         * 
         * \param[in] inputSources get the dimensions of the input sources. The inputSources is not copied to allow dupplication of sources, for example with parallelism.
         */
        virtual void setInputDimensions(const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) override;

        /**
         * \brief return the genotype template an individual.
         */
        virtual std::vector<Node::NodeValueTemplate> getGenotypeTemplate() const;

        /**
         * \brief individual nodes should have six values, with limited ranges.
         * 
         * \param[in] indiv Individual controlled.
         */
        virtual bool isValid(const Evolution::Individual& indiv) override;

        /**
         * \brief execute each node as an instruction line. 
         * 
         * A node is of shape: {a, b, c, d, e, f}.
         *  - "a" is the index of the output register.
         *  - "b" is the index of the executed function.
         *  - "c" is indicates the first input type (register, state value, weight...).
         *  - "d" is the index of the first input.
         *  - "e" is indicates the second input type (register, state value, weight...).
         *  - "f" is the index of the second input.
         * 
         * \param[in] indiv Individual executed
         * \param[in] inputSources input sources on which the individual is executed.
         */
        virtual std::vector<double> executeIndividual(
            const Evolution::Individual& indiv, const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) override;

        };
}; // namespace LGP_Representation


#endif
