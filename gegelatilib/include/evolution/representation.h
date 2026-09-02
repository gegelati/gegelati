
#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include <memory>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include "iostream"

#include "evolution/individual.h"
#include "evolution/population.h"
#include "node/genotypeTemplate.h"
#include "representation/repParameters.h"
#include "newData/dataValue.h"

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

        /// Input dimensions
        std::vector<Data::DataType> inputDimensions;

        /// Output dimension
        Data::DataType outputDimension;

        /// Name of the representation.
        std::string representationName = "";
        /// Color of the representation.
        std::string representationColor = "";

        /// True if represnetation allows tangled connections.
        bool tangled = false;

        /// Tangled population
        std::optional<std::reference_wrapper<const Population>> tangledPopulation;

      public:

      
        /// Default polymorphic destructor
        virtual ~Representation() = default;
        /// @brief clone pattern 
        virtual std::unique_ptr<Representation> cloneUniquePtr() const = 0;


        /**
         * \brief Main Representation constructor.
         * 
         * \param[in] nbNodesMin the minimum number of nodes in the representation.
         * \param[in] nbNodesMax the maximum number of nodes in the representation.
         * \param[in] representationName name of the representation used.
         * \param[in] representationColor color of the representation used (during .dot files).
         */
        Representation(
            size_t nbNodesMin, size_t nbNodesMax=0,
            std::string representationName = "Representation", 
            std::string representationColor = "#000000")
               : nbNodesMin{nbNodesMin}, nbNodesMax{nbNodesMax}, representationName(representationName), representationColor(representationColor) {
                if(this->nbNodesMax == 0) {
                    this->nbNodesMax = this->nbNodesMin;
                }
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

        /**
         * \brief set the dimensions
         * 
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] outputDimension the dimensions of the output source.
         */
        virtual void setDimensions(const std::vector<Data::DataType>& inputDimensions, const Data::DataType& outputDimension);

        /**
         * \brief get the input dimensions of the representation.
         */
        virtual const std::vector<Data::DataType>& getInputDimensions() const;

        /**
         * \brief get the output dimension of the representation.
         */
        virtual const Data::DataType& getOutputDimension() const;

        /**
         * \brief return the genotype template an individual.
         */
        virtual std::unique_ptr<const Node::GenotypeTemplate> getGenotypeTemplate() const = 0;

        /**
         * \brief identified wether the individual is valid faced to the expected node structure of the representation.
         * 
         * \param[in] indiv individual controlled.
         */
        virtual bool isValid(const Individual& indiv) const = 0;

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
        virtual Data::DataValue executeIndividual(
            const Individual& indiv, const std::vector<Data::DataView>& inputSources) const  = 0;
    };
}; // namespace Representation

#endif // REPRESENTATION_H
