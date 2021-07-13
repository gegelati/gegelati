#include "data/dataHandlerPrinter.h"

std::string Data::DataHandlerPrinter::printDataAt(
    const std::type_info& type, const size_t address,
    const std::string& nameVar) const
{
    const std::type_info& templateType = dataHandler->getTemplateType();
    const std::vector<size_t> dataSizes{dataHandler->getDimensionsSize()};

    // Check if the operand need only one value
    if (type == templateType) {
        return std::string{" = " + nameVar + "[" + std::to_string(address) +
                           "];"};
    }
    // else if(false){}
    const std::vector<size_t> opDimension = this->getOperandSizes(type);
    std::string operandInit;
    if (dataSizes.size() == 1) {
        // We retreive all indexes of the global variable to initialize
        // the operand for a 1D array
        operandInit = "[] = ";

        size_t operandSize = opDimension.at(0);

        operandInit += print1DArray(address, operandSize, nameVar);
    }
    else if (dataSizes.size() == 2) {
        if (opDimension.size() == 1) {
            operandInit = "[] = ";
            operandInit += print1DArray(address, opDimension.at(0), nameVar);
        }
        else {
            operandInit = "[][] = ";
            operandInit +=
                print2DArray(address, dataSizes, opDimension, nameVar);
        }
    }
    operandInit += ";";
    return operandInit;
}

std::string Data::DataHandlerPrinter::getTemplatedType() const
{
    return std::string{
        DEMANGLE_TYPEID_NAME(dataHandler->getTemplateType().name())};
}

std::string Data::DataHandlerPrinter::print1DArray(
    const size_t& start, const size_t& size, const std::string& nameVar) const
{
    std::string array{"{"};
    int end = start + size;
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
    const std::vector<size_t>& generatedTabSize, const std::string& nameVar) const
{

    return std::__cxx11::string();
}

std::vector<size_t> Data::DataHandlerPrinter::getOperandSizes(
    const std::type_info& type) const
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
            // std::cout<< e.what() << " try to convert : " << shrink
            // <<std::endl;
        }
    }
    return sizes;
}
