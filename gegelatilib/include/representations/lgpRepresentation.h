
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

            /// The number of registers required to output a value.
            size_t nbOutputRegisters;

            /// The number of registers used by the LGPs
            size_t nbRegisters;



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
             * \param[in] genotype genotype executed
             * \param[in] inputSources input sources on which the individual is executed.
             */
            virtual Data::DataValue executeGenotype(
                const Evolution::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const override;

            /// @brief Unique pointer of the genotypeGenerator, as it is fixed during evolution
            std::unique_ptr<Node::GenotypeGenerator> genotypeGenerator;

            /**
             * \brief Create the genotype constraint grammar
             */
            virtual void setGenotypeConstraint() override;

            
            /**
             * \brief Create the genotype generator
             */
            virtual void setGenotypeGenerator();

        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Evolution::Representation> cloneUniquePtr() const override;

            /**
             * \brief Main Representation constructor.x
             * 
             * \param[in] inputDimensions for the representation
             * \param[in] nbOutputRegisters number of output registers to create the outputDimension
             * \param[in] iSet the Instruction Set used by the LGPRepresentation.
             * \param[in] nbRegisters the number of registers used by the LGPs
             * \param[in] nbNodesMin the minimum number of nodes in the representation.
             * \param[in] nbNodesMax the maximum number of nodes in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            LGPRepresentation(std::vector<Dimensions::Requirement> inputDimensions, size_t nbOutputRegisters, const Instructions::Set& iSet, size_t nbRegisters, size_t nbNodesMin, size_t nbNodesMax=0, std::string representationName = "LGP", std::string representationColor = "#922DB4")
                : Evolution::Representation(
                    inputDimensions, Dimensions::Requirement::array1d<double>(nbOutputRegisters, Dimensions::NumericRange<double>::unbounded()), 
                    nbNodesMin, nbNodesMax, representationName, representationColor), iSet{iSet}, nbOutputRegisters{nbOutputRegisters},
                    nbRegisters{nbRegisters}{
                if(nbOutputRegisters > nbRegisters) {
                    throw std::runtime_error("LGPRepresentation::Constructor: Number of outputRegisters cannot be higher than the number of registers");
                }
                this->setGenotypeConstraint();
                this->setGenotypeGenerator();
            };

            /**
             * \brief return the genotype template an LGP individual, defined in setGenotypeTemplate.
             */
            virtual std::unique_ptr<Node::GenotypeGenerator> getGenotypeGenerator() const override;

    };
}; // namespace LGP_Representation


#endif
