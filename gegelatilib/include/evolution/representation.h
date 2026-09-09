
#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include <memory>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include <functional>
#include "iostream"

#include "evolution/individual.h"
#include "evolution/population.h"
#include "node/genotypeTemplate.h"
#include "representation/repParameters.h"
#include "data/dataValue.h"
#include "dimensions/dimensionFlow.h"
#include "dimensions/activationFunctions.h"

using RepParam = Representation::RepresentationParameters;

namespace Evolution {
    /**
     * \brief Abstract class representing an Representation.
     *
     * Available representations are TPG, MAPLE, and LGP
     */
    class Representation
    {
      protected:

        /// Minimum Number of nodes in the representation.
        size_t nbNodesMin = 0;

        /// Maximum number of nodes in the representation.
        size_t nbNodesMax = 0;

        /// Name of the representation.
        std::string representationName = "";
        /// Color of the representation.
        std::string representationColor = "";

        /// True if represnetation allows tangled connections.
        bool tangled = false;

        /// Tangled population
        std::optional<std::reference_wrapper<const Population>> tangledPopulation;

        /// @brief Vector of output functions
        std::vector<std::unique_ptr<Dimensions::ActivationFunctions::Function>> outputFunctions;

        /// @brief Dimension flow
        Dimensions::DimensionFlow dimensionFlow;

        /**
         * \brief private method for executing individual
         */
        virtual Data::DataValue executeIndividualRaw(
          const Individual& indiv, const std::vector<Data::DataView>& inputSources) const = 0;

      public:

      
        /// Default polymorphic destructor
        virtual ~Representation() = default;
        /// @brief clone pattern 
        virtual std::unique_ptr<Representation> cloneUniquePtr() const = 0;


        /**
         * \brief Main Representation constructor.
         * 
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] outputDimension the dimension of the output.
         * \param[in] nbNodesMin the minimum number of nodes in the representation.
         * \param[in] nbNodesMax the maximum number of nodes in the representation.
         * \param[in] representationName name of the representation used.
         * \param[in] representationColor color of the representation used (during .dot files).
         */
        Representation(
            const std::vector<Dimensions::Requirement>& inputDimensions, const Dimensions::Requirement& outputDimension,
            size_t nbNodesMin, size_t nbNodesMax=0,
            std::string representationName = "Representation", 
            std::string representationColor = "#000000")
               : nbNodesMin{nbNodesMin}, nbNodesMax{nbNodesMax}, representationName(representationName), 
                 representationColor(representationColor), dimensionFlow{inputDimensions} {
                if(this->nbNodesMax == 0) {
                    this->nbNodesMax = this->nbNodesMin;
                }

                // Add layer
                dimensionFlow.addLayer(representationName, inputDimensions, outputDimension);
            };   

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Representation(const Representation&) = delete;
        Representation& operator=(const Representation&) = delete;

        /**
         * \brief Return the name of the representation.
         */
        std::string getRepresentationName() const { return this->representationName; }
        /**
         * \brief Return the color of the representation.
         */
        std::string getRepresentationColor() const { return this->representationColor; }

        /**
         * \brief get the minimum number of nodes
         */
        virtual size_t getMinNbNodes() const;

        /**
         * \brief get the maximum number of nodes
         */
        virtual size_t getMaxNbNodes() const;

        /** \brief Adds a typed post-processing function to the representation output. */
        virtual void addOutputFunction(std::unique_ptr<Dimensions::ActivationFunctions::Function> function);

        /**
         * \brief Return the current dimension flow
         */
        virtual Dimensions::DimensionFlow getDimensionFlow() const;

        /**
         * \brief print the summary of the dimension flow
         */
        virtual std::string summary() const;

        /**
         * \brief return the genotype template an individual.
         */
        virtual std::unique_ptr<Node::GenotypeTemplate> getGenotypeTemplate() const = 0;

        /**
         * \brief identified wether the individual is valid faced to the expected node structure of the representation.
         * 
         * \param[in] indiv individual controlled.
         */
        virtual bool isValid(const Individual& indiv) const;

        /**
         * \brief set the statue of the tangled property
         * 
         * \param[in] tangled statue of the tangled property
         */
        virtual void setTangled(bool tangled);

        /**
         * \brief get if the representation is tangled
         */
        virtual bool isTangled() const;

        /**
         * Set a tangled population
         * 
         * \param[in] tangledPop population set
         */
        virtual void setTangledPopulation(const Population& tangledPop);

        /**
         * \brief return if representation has a tangled population set.
         */
        virtual bool hasTangledPopulation();

        /**
         * \brief return the tangled population(can be null)
         */
        virtual const std::optional<std::reference_wrapper<const Population>>& getTangledPopulation();

        /**
         * \brief execute the specified individual based on the current dataSources
         * 
         * \param[in] indiv Individual executed
         * \param[in] inputSources input sources on which the individual is executed.
         */
        Data::DataValue executeIndividual(
          const Individual& indiv, const std::vector<Data::DataView>& inputSources) const;
    };
}; // namespace Representation

#endif // REPRESENTATION_H
