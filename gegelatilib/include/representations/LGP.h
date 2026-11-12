
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
            Instructions::Set iSet;

            /// The number of registers required to output a value.
            Data::DataType outputRegistersType;

            /// The register values dupplicated during LGP individual execution
            Data::DataType registersType;

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

            /**
             * \brief Create the genotype constraint grammar
             */
            virtual void setGenotypeConstraint() override;

            

            /// \brief the minimum number of lines in the representation.
            size_t nbLinesMin;
            /// \brief the maximum number of lines in the representation.
            size_t nbLinesMax;

            /**
             * \brief Define the largest address space reachable by the provided instructions based on the input types.
             */
            void setLargestAddressSpace();

            /// @brief Largest address space of the representation
            size_t largestAddressSpace;
        public:

            /**
             * \brief get a const reference toward the current instruction set.
             */
            const Instructions::Set& getInstructionSet();

            /// @brief clone pattern 
            virtual std::unique_ptr<Representation> cloneOnlyRepresentation() const override;

            /// Inherited from Named, adds the number of lines and of registers.
            virtual std::string getDetails() const override
            {
                return this->getName() + "(" + std::to_string(this->nbLinesMin) + " to " + std::to_string(this->nbLinesMax) + " lines, "
                       + std::to_string(this->registersType.totalElements()) + " registers)";
            }

            /**
             * \brief Main Representation constructor.x
             * 
             * \param[in] inputDimensions for the representation
             * \param[in] registersType the register used by the LGP.
             * \param[in] iSet the Instruction Set used by the LGP.
             * \param[in] outputRegistersType number of output registers to create the outputDimension
             * \param[in] nbLinesMin the minimum number of lines in the representation.
             * \param[in] nbLinesMax the maximum number of lines in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            LGP(const std::vector<Dimensions::Requirement>& inputDimensions, const Data::DataType& registersType, const Instructions::Set& iSet, const Data::DataType& outputRegistersType, size_t nbLinesMin, size_t nbLinesMax = 0, std::string representationName = "LGP", std::string representationColor = "#922DB4")
                : Representation(
                    inputDimensions, Dimensions::Requirement(outputRegistersType, Dimensions::NumericRange<double>::unbounded()), 
                    representationName, representationColor), outputRegistersType{outputRegistersType},
                    registersType{registersType}, nbLinesMin{nbLinesMin}, nbLinesMax{nbLinesMax} {
                if(outputRegistersType.totalElements() > this->registersType.totalElements()) {
                    throw std::runtime_error("LGP::Constructor: Number of outputRegisters cannot be higher than the number of registers");
                }
                if(nbLinesMax == 0) {
                    this->nbLinesMax = nbLinesMin;
                }

                std::vector<Data::DataType> inputTypes{this->registersType};
                for(const Dimensions::Requirement& req: inputDimensions) {
                    inputTypes.push_back(req.getDataType());
                }
                this->iSet = iSet.filterInstructionSet(inputTypes, this->registersType);
                if(this->iSet.getNbInstructions() == 0) {
                    throw std::runtime_error("LGP:Constructor: No instruction can be used with current inputs");
                }

                this->setLargestAddressSpace();
                this->setGenotypeConstraint();
            };

            /**
             * \brief Identify the intron nodes: the nodes that does not write in any registers.
             */
            void detectIntronNodes(GraphBased::Genotype& genotype) const override;

    };
}; // namespace LGP_Representation


#endif // REPRESENTATION_LGP_H
