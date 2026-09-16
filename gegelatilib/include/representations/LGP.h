
#ifndef REPRESENTATIONS_LGP_H
#define REPRESENTATIONS_LGP_H

#include <memory>
#include <vector>

#include "representations/representation.h"
#include "instructions/set.h"

 namespace Representations {

    /**
     * \brief Abstract class representing a LGP
     */
    class LGP : public Representation
    {
        protected:

            /// Instruction Set used by the LGP
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
                const GraphBased::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const override;

            /// @brief Unique pointer of the genotypeGenerator, as it is fixed during evolution
            std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator;

            /**
             * \brief Create the genotype constraint grammar
             */
            virtual void setGenotypeConstraint() override;

            
            /**
             * \brief Create the genotype generator
             */
            virtual void setGenotypeGenerator();

            /// \brief the minimum number of lines in the representation.
            size_t nbLinesMin;
            /// \brief the maximum number of lines in the representation.
            size_t nbLinesMax;
        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Representation> cloneOnlyRepresentation() const override;

            /**
             * \brief Main Representation constructor.x
             * 
             * \param[in] inputDimensions for the representation
             * \param[in] nbOutputRegisters number of output registers to create the outputDimension
             * \param[in] iSet the Instruction Set used by the LGP.
             * \param[in] nbRegisters the number of registers used by the LGPs
             * \param[in] nbLinesMin the minimum number of lines in the representation.
             * \param[in] nbLinesMax the maximum number of lines in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            LGP(std::vector<Dimensions::Requirement> inputDimensions, size_t nbOutputRegisters, const Instructions::Set& iSet, size_t nbRegisters, size_t nbLinesMin, size_t nbLinesMax = 0, std::string representationName = "LGP", std::string representationColor = "#922DB4")
                : Representation(
                    inputDimensions, Dimensions::Requirement::array1d<double>(nbOutputRegisters, Dimensions::NumericRange<double>::unbounded()), 
                    representationName, representationColor), iSet{iSet}, nbOutputRegisters{nbOutputRegisters},
                    nbRegisters{nbRegisters}, nbLinesMin{nbLinesMin}, nbLinesMax{nbLinesMax} {
                if(nbOutputRegisters > nbRegisters) {
                    throw std::runtime_error("LGP::Constructor: Number of outputRegisters cannot be higher than the number of registers");
                }
                if(nbLinesMax == 0) {
                    this->nbLinesMax = nbLinesMin;
                }
                this->setGenotypeConstraint();
                this->setGenotypeGenerator();
            };

            /**
             * \brief return the genotype template an LGP individual, defined in setGenotypeTemplate.
             */
            virtual std::unique_ptr<GraphBased::GenotypeGenerator> getGenotypeGenerator() const override;

    };
}; // namespace LGP_Representation


#endif // REPRESENTATION_LGP_H
