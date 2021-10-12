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
#include <sstream>

#include "data/dataHandlerPrinter.h"

std::string Data::DataHandlerPrinter::printDataAt(
    const Data::DataHandler& dataHandler, const std::type_info& type,
    const size_t& address, const std::string& nameVar) const
{
    const std::type_info& templateType = dataHandler.getNativeType();
    const std::vector<size_t> dataSizes{dataHandler.getDimensionsSize()};

    // Check if the operand need only one value
    if (type == templateType) {
        return std::string{" = " + nameVar + "[" + std::to_string(address) +
                           "];"};
    }

    std::vector<size_t> opDimension{getOperandSizes(type)};
    std::string operandInit;
    if (opDimension.size() <= dataSizes.size()) {
        switch (dataSizes.size()) {
        case 1:
            operandInit = "[] = ";

            operandInit += print1DArray(address, opDimension.at(0), nameVar);

            break;
        case 2:
            for (auto elmt : opDimension) {
                operandInit += "[" + std::to_string(elmt) + "]";
            }
            operandInit += " = ";

            if (opDimension.size() == 2) {
                operandInit +=
                    print2DArray(address, dataSizes, opDimension, nameVar);
            }
            else {

                size_t width = dataSizes.at(0);
                size_t arrayWidth = opDimension.at(0);

                size_t addressH = address / (width - arrayWidth + 1);
                size_t addressW = address % (width - arrayWidth + 1);
                size_t addressSrc = (addressH * width) + addressW;
                operandInit += print1DArray(addressSrc, arrayWidth, nameVar);
            }
            break;
        default:
            throw std::invalid_argument(
                "DataHandlerPrinter only manage 1D and 2D DataHandler.");
        }
    }
    else {
        throw std::invalid_argument(
            "The operand require a data source with an higher dimension.");
    }

    operandInit += ";";
    return operandInit;
}

std::string Data::DataHandlerPrinter::getDemangleTemplateType(
    const Data::DataHandler& dataHandler) const
{
    return std::string{
        DEMANGLE_TYPEID_NAME(dataHandler.getNativeType().name())};
}

std::string Data::DataHandlerPrinter::print1DArray(
    const size_t& start, const size_t& size, const std::string& nameVar) const
{
    std::string array{"{"};
    size_t end = start + size;
    for (size_t idx = start; idx < end; ++idx) {
        array += (nameVar + "[" + std::to_string(idx) + "]");
        if (idx < (end - 1)) {
            array += ", ";
        }
    }
    array += "}";
    return array;
}

std::string Data::DataHandlerPrinter::print2DArray(
    const size_t& start, const std::vector<size_t>& sourceTabSize,
    const std::vector<size_t>& generatedTabSize,
    const std::string& nameVar) const
{
    std::string array{"{"};
    size_t width = sourceTabSize.at(1);

    size_t arrayHeight = generatedTabSize.at(0);
    size_t arrayWidth = generatedTabSize.at(1);

    size_t addressH = start / (width - arrayWidth + 1);
    size_t addressW = start % (width - arrayWidth + 1);
    size_t addressSrc = (addressH * width) + addressW;
    size_t idxSrc;

    for (size_t idxHeight = 0; idxHeight < arrayHeight; idxHeight++) {
        idxSrc = addressSrc + idxHeight * width;
        array += print1DArray(idxSrc, arrayWidth, nameVar);

        if (idxHeight < (arrayHeight - 1)) {
            array += ", ";
        }
    }
    array += "}";
    return array;
}

std::vector<size_t> Data::DataHandlerPrinter::getOperandSizes(
    const std::type_info& type)
{
    std::vector<size_t> sizes;
    std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
    std::string token;
    std::istringstream tokenStream(typeName);

    while (std::getline(tokenStream, token, '[')) {
        std::string shrink{token.substr(0, token.length() - 1)};
        try {
            sizes.push_back(std::stoull(shrink));
        }
        catch (std::exception const& e) {
        }
    }
    return sizes;
}
#endif // CODE_GENERATION
