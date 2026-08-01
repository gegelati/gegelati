/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

//#ifdef CODE_GENERATION

#ifndef CODE_GENERATION_ENGINE_H
#define CODE_GENERATION_ENGINE_H
#include <ios>
#include <iostream>
#include <string>

#include "evoGraph/abstractEngine.h"
#include "evoGraph/edge.h"
#include "evoGraph/graph.h"
#include "evoGraph/team.h"
#include "representation/individual.h"
#include "representation/representation.h"

namespace CodeGen {
    /**
     * \brief Class in charge of generating the C code of a Graph.
     *
     * Each program of the Graph is represented by a C function.
     * All the functions are regrouped in a file. Another file holds
     * the required functions to iterate through the Graph.
     *
     * To use the generated code two code templates are provided in the
     * directory doc/codeGen. One template is for generic learning environment.
     * The other one is dedicated for adversarial learning environment and
     * manages the switch between the players. Both templates can use the
     * inference with the codeGen or the inference with Gegelati.
     *
     * The repo gegelati apps give some example of the template code completed
     * for TicTacToe, Pendulum and StickGame.
     */
    class CodeGenerationExporter : public EvoGraph::AbstractEngine
    {
      protected:

        /// File holding the functions in charge of iterating through the individual.
        std::ofstream fileMain;
        /// File holding the header functions.
        std::ofstream fileMainH;


        /**
         * \brief function printing activation function code in the C files.
         *
         * This function print the activation function in the param file of the
         * Environment instance.
         */
        virtual void initActivationFunction(const Representation::Representation& representation);

      public:
        /**
         * \brief Main constructor of the class.
         *
         * \param[in] filename : filename of the file holding the main function
         *                of the generated program.
         *
         * \param[in] subAlgoNames names of the sub representations
         * \param[in] path to the folder in which the file are generated. If the
         * folder does not exist.
         */
        CodeGenerationExporter(const std::string& filename, std::vector<std::string> subAlgoNames,
                            const std::string& path = "./");

        /**
         * \brief destructor of the class.
         *
         * add endif at the end of the header and close both file.
         */
        virtual ~CodeGenerationExporter();

        /**
         * \brief function that creates the C files required to execute the Individual
         * without gegelati.
         *
         * This function iterates trough the Graph and create the required C
         * code to represent each element of the Graph.
         * 
         * \param[in] individual the individual generated
         * \param[in] representation the representation corresponding to the individual
         * \param[in] subIndividuals map of the sub individuals plot during the exporting of the individual
         */
        virtual void exportMainIndividual(const Representation::Individual& individual, const Representation::Representation& representation, std::map<uint64_t, std::set<std::reference_wrapper<const Representation::Individual>>>& subIndividuals);

        
        /**
         * \brief function that creates the C files required to execute the Individual
         * without gegelati.
         *
         * This function iterates trough the Graph and create the required C
         * code to represent each element of the Graph.
         * 
         * \param[in] individuals the individuals generated
         * \param[in] representation the representation corresponding to the individual
         * \param[in] subIndividuals map of the sub individuals plot during the exporting of the individual
         */
        virtual void exportIndividuals(std::set<std::reference_wrapper<const Representation::Individual>> individuals, const Representation::Representation& representation, std::map<uint64_t, std::set<std::reference_wrapper<const Representation::Individual>>>& subIndividuals);
    };
} // namespace CodeGen

#endif // TPGGENERATIONENGINE_H

//#endif // CODE_GENERATION
