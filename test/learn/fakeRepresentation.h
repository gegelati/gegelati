#ifndef FAKE_REPRESENTATION_H
#define FAKE_REPRESENTATION_H


#include "evolution/representation.h"

namespace Representations {

    /**
     * \brief Fake rep
     */
    class FakeRepresentation : public Evolution::Representation
    {
        protected:

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
                const Evolution::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const override {
                    return Data::DataValue::scalar<int>(0);
                }

            
            virtual void setGenotypeConstraint() override {
                Node::NodeConstraint nc;
                nc.addConstraint(Dimensions::UnconstrainedData().cloneSharedPtr());
                this->genotypeConstraint = std::make_unique<Node::GenotypeConstraint>(nc, 1, 1);
            }
    
        public:

            /// @brief clone pattern 
            virtual std::unique_ptr<Evolution::Representation> cloneUniquePtr() const override {
                return std::make_unique<FakeRepresentation>();
            };

            /**
             * \brief Main FakeRepresentation constructor.
             * 
             * \param[in] inputDimensions the dimensions of the input sources.
             * \param[in] outputDimension The number of actions in the TPGRepresentation.
             * \param[in] nbNodesMin the minimum number of nodes in the representation.
             * \param[in] nbNodesMax the maximum number of nodes in the representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            FakeRepresentation(std::vector<Dimensions::Requirement> inputDimensions = {Dimensions::Requirement::scalar<int>()},
                               Dimensions::Requirement outputDimension = Dimensions::Requirement::scalar<int>(),
                               size_t nbNodesMin=5, size_t nbNodesMax=0,
                               std::string representationName = "FakeRepresentation", 
                               std::string representationColor = "#FFFFFF")
                : Evolution::Representation(
                    inputDimensions, outputDimension, nbNodesMin, nbNodesMax, representationName, representationColor){
                        this->setGenotypeConstraint();
                    };

            /**
             * \brief return the genotype template an LGP individual, defined in setGenotypeTemplate.
             */
            virtual std::unique_ptr<Node::GenotypeGenerator> getGenotypeGenerator() const override{
                Node::NodeGenerator ng;
                ng.addGenerator(Dimensions::NumericUniformGenerator<int>(0, 0).cloneUniquePtr());
                return std::make_unique<Node::GenotypeGenerator>(ng, 1, 1);
            };

            std::unique_ptr<Node::GenotypeConstraint>& getGenotypeConstraintMut() {
                return this->genotypeConstraint;
            }

        };
}; // namespace LGP_Representation


#endif
