
#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include <memory>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include "iostream"

#include "evolution/individual.h"
#include "representation/repParameters.h"

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

        /// Name of the representation.
        std::string representationName;
        /// Color of the representation.
        std::string representationColor;

      public:

      
        /// Default polymorphic destructor
        virtual ~Representation() = default;


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

    };
}; // namespace Representation

#endif // REPRESENTATION_H
