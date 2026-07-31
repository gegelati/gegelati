
#ifndef TGP_REPRESENTATION_H
#define TGP_REPRESENTATION_H

#include <memory>
#include <vector>

#include "representation/lgp/environment.h"

#include "representation/lgp/lgpRepresentation.h"
#include "representation/tgp/tgpMutator.h"
namespace Representation::TGP {

    /**
     * \brief Class representing a TreeBased GPRepresentation
     * 
     * For now, this class does not support instruction with types different from the environment (ex env with int data and instrunctions with double inputs)
     */
    class   TGPRepresentation : public LGP::LGPRepresentation
    {
        public:

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] iSet the Instruction Set used by the LGPRepresentation.
             * \param[in] parameters the LearningParameters used by the Representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            TGPRepresentation(const Instructions::Set& iSet, std::unique_ptr<RepresentationParameters> parameters = std::make_unique<RepresentationParameters>(), std::string representationName = "TGP", std::string representationColor = "#d37217")
                : LGP::LGPRepresentation(iSet, std::move(parameters), representationName, representationColor) {};

            /**
             * \brief Initialize the mutator of the representation
             */
            virtual void initMutator() override;

            /**
             * \brief Initialize the representation.
             */
            virtual void initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;
            /**
             * Copy and return a uniqure pointer of the representation
             */
            virtual std::unique_ptr<Representation> copy() const override;
        };
}; // namespace TGP::Representation


namespace Representation{
    /**
     * To make the aglrotihm accessible from representation namespace
     */
    using TGPRepresentation = TGP::TGPRepresentation;
}

#endif
