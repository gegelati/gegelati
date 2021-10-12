/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2021)
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

#ifdef CODE_GENERATION
#ifndef DATA_HANDLER_PRINTER_H
#define DATA_HANDLER_PRINTER_H

#include "data/dataHandler.h"
#include "data/demangle.h"

namespace Data {
    /**
     * \brief Class used to generate the declaration of operands of a line for
     * the code gen.
     */
    class DataHandlerPrinter
    {
      public:
        /// Constructor for the DataHandlerPrinter
        DataHandlerPrinter() = default;

        /// destructor
        virtual ~DataHandlerPrinter() = default;

        /// copy constructor
        DataHandlerPrinter(const DataHandlerPrinter& other) = default;

        /**
         * \brief Function used to generate the declaration of an operand based
         * on its type.
         *
         * \param[in] dataHandler const reference to the Data::DataHandler that
         * is accessed by the printed code.
         * \param[in] type the std::type_info of the operand that we want to
         * print.
         * \param[in] address the location of the data to print.
         * \param[in] nameVar the name of the source global variable in the
         * generated code.
         *
         * \return The end of the declaration of the operand of type : type and
         * its initialization based on the extracted data of the global variable
         * nameVar at index address.
         */
        std::string printDataAt(const Data::DataHandler& dataHandler,
                                const std::type_info& type,
                                const size_t& address,
                                const std::string& nameVar) const;

        /**
         * \brief Function that return the initialization of a 1D array.
         *
         * \param[in] start the address of the first element of the array to
         * extract.
         * \param[in] size the size of the array to print.
         * \param[in] nameVar the name of the global variable in which the array
         * have to be extracted.
         *
         * \return The initialization of a 1D array of size size extrated of the
         * global variable nameVar at address start.
         */
        std::string print1DArray(const size_t& start, const size_t& size,
                                 const std::string& nameVar) const; // override;

        /**
         * \brief Function that return the initialization of a 2D array.
         *
         * This function generates generatedTabSize[1] (the height of the
         * operand) 1D array by calling the function print1DArray.
         *
         * \param[in] start the address of the first element of the array to
         * extract.
         * \param[in] sourceTabSize a const reference to a std::vector
         * containing the height and the width of the global variable.
         * \param[in] generatedTabSize a const reference to a std::vector
         * containing the height and the width of the operand that has to be
         * printed.
         * \param[in] nameVar the name of the global variable in which
         * the array have to be extracted.
         *
         * \return The initialization of a 2D array of size generatedTabSize
         * extracted of the global variable nameVar at address start.
         */
        std::string print2DArray(const size_t& start,
                                 const std::vector<size_t>& sourceTabSize,
                                 const std::vector<size_t>& generatedTabSize,
                                 const std::string& nameVar) const;

        /**
         * \brief function used to retrieve the typename of the template of the
         * DataHandler.
         *
         * \param[in] dataHandler const reference to the Data::DataHandler that
         * must be printed
         * \return template type of the DataHandler in a human readable format.
         */
        std::string getDemangleTemplateType(
            const Data::DataHandler& dataHandler) const;

        /**
         * \brief Function that return the size of each dimension of an operand.
         *
         * \param[in] type the std::type_info of the operand.
         * \return a std::vector containing the size of each dimension of the
         * operand of type type.
         */
        static std::vector<size_t> getOperandSizes(const std::type_info& type);
    };

} // namespace Data

#endif // DATAHANDLERPRINTER_H
#endif // CODE_GENERATION
