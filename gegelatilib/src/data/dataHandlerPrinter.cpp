#include "data/dataHandlerPrinter.h"

std::string Data::DataHandlerPrinter::printDataAt(
    const std::type_info& type, const size_t address,
    const std::string& nameVar) const
{
    const std::type_info& templateType = dataHandler->getTemplateType();

    // Check if the operand need only one value
    if (type == templateType) {
        return std::string{" = " + nameVar + "[" + std::to_string(address) +
                           "];"};
    }
    //else if(false){}

    // Else, we retreive all indexes of the global variable to initialize
    // the operand for a 1D array
    std::string operandInit{"[] = "};
    std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
    std::string regex{this->getTemplatedType()};
    regex.append("\\s*(const\\s*)?\\[([0-9]+)\\]");
    std::regex arrayType(regex);
    std::cmatch cm;
    size_t operandSize = 0;
    std::cout << "expression où on cherche la regex : " << typeName << std::endl;
    if (std::regex_match(typeName.c_str(), cm, arrayType)) {

        std::cout << "nombre de match dans cm : " << cm.size() << std::endl;
        operandSize = std::atoi(cm[2].str().c_str());
    }
    operandInit += print1DArray(address, operandSize, nameVar);

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