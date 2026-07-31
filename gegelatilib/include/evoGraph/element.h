/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef GRAPH_ELEMENT_H
#define GRAPH_ELEMENT_H

#include <optional>
#include <memory>
#include <stdexcept>
#include <cinttypes>
#include <iostream>
#include <list>
#include <set>

namespace Representation{
  class Individual;
}

namespace EvoGraph {
    /**
     * \brief Abstract class representing the elements of a Graph, either vertices or edges
     * 
     * An element can contain an agent.
     */
    class Element
    {
      public:
        /// Default polymorphic destructor
        virtual ~Element() = default;


        /**
         * \brief Get a const weak pointer of the Individual Program of the Element.
         *
         * \return a const weak pointer of the Individual Program of the Element.
         */
        virtual const Representation::Individual& getProgram() const;

        /**
         * \brief Set a new Individual Program for the Element.
         *
         * \param[in] agentProgram the new weak pointer to a Individual Program.
         */
        virtual void setProgram(const Representation::Individual& agentProgram);

        /**
         * \brief remove the program of the agent by setting nullopt.
         */
        virtual void removeProgram();

        /**
         * \brief return true if the element has a program agent
         */
        virtual bool hasProgram() const;


      protected:

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;

        /**
         * \brief Protected default constructor to forbid the instanciation of
         * object of this abstract class.
         * 
         * \param[in] agentProgram the shared pointer to the Individual Program associated to the
         *            Element.
         */
        Element(std::optional<std::reference_wrapper<const Representation::Individual>> agentProgram = std::nullopt) : program{agentProgram}{};


        /// Shared pointer to the Individual to execute when evaluating the bid
        /// of this Edge.
        /// This attribute is mutable to enable its modification during
        /// mutations.
       std::optional<std::reference_wrapper<const Representation::Individual>> program;
    };
}; // namespace EvoGraph

#endif
