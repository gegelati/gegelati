
#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include <memory>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include "iostream"

#include "evolution/individual.h"
#include "node/genotypeTemplate.h"
#include "representation/repParameters.h"
#include "data/primitiveTypeArray.h"

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
        size_t nbNodesMin;

        /// Maximum number of nodes in the representation.
        size_t nbNodesMax;

        /// Number of input sources
        size_t nbInputSources;

        /// Maximum index of input source
        size_t maxInputSourceIdx;

        /// Name of the representation.
        std::string representationName;
        /// Color of the representation.
        std::string representationColor;

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
         * \brief set the number of input sources
         * 
         * \param[in] inputSources get the dimensions of the input sources. The inputSources is not copied to allow dupplication of sources, for example with parallelism.
         */
        virtual void setInputDimensions(const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources);

        /**
         * \brief get the number of input sources
         */
        virtual size_t getNbInputSources() const;

        /**
         * \brief get the maximum index of input source
         */
        virtual size_t getMaxInputSourceIdx() const;

        /**
         * \brief return the genotype template an individual.
         */
        virtual std::unique_ptr<const Node::GenotypeTemplate> getGenotypeTemplate() const = 0;

        /**
         * \brief identified wether the individual is valid faced to the expected node structure of the representation.
         * 
         * \param[in] indiv individual controlled.
         */
        virtual bool isValid(const Individual& indiv) = 0;

        /**
         * \brief execute the specified individual based on the current dataSources
         * 
         * \param[in] indiv Individual executed
         * \param[in] inputSources input sources on which the individual is executed.
         */
        virtual std::vector<double> executeIndividual(
            const Individual& indiv, const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) const  = 0;
    };
}; // namespace Representation

#endif // REPRESENTATION_H
