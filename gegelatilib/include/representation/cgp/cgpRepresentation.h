
#ifndef CGP_REPRESENTATION_H
#define CGP_REPRESENTATION_H

#include <memory>
#include <vector>

#include "representation/lgp/environment.h"

#include "representation/lgp/lgpRepresentation.h"
#include "representation/cgp/cgpMutator.h"
namespace Representation::CGP {

    /**
     * \brief Class representing a CGPRepresentation
     * 
     * For now, this class does not support instruction with types different from the environment (ex env with int data and instrunctions with double inputs)
     */
    class   CGPRepresentation : public LGP::LGPRepresentation
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
            CGPRepresentation(const Instructions::Set& iSet, std::unique_ptr<RepresentationParameters> parameters = std::make_unique<RepresentationParameters>(), std::string representationName = "CGP", std::string representationColor = "#98a02c")
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
}; // namespace CGP::Representation


namespace Representation{
    /**
     * To make the aglrotihm accessible from representation namespace
     */
    using CGPRepresentation = CGP::CGPRepresentation;
}

#endif
