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