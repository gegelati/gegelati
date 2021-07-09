#ifdef CODE_GENERATION
#ifndef GEGELATI_DATAHANDLERPRINTER_H
#define GEGELATI_DATAHANDLERPRINTER_H

#include "arrayWrapper.h"
#include "dataHandler.h"

namespace Data {
    /**
     * DataHandlerPrinter
     *
     * // todo
     *
     * @tparam T
     */
    template <typename T> class DataHandlerPrinter
    {
        static_assert(std::is_base_of<Data::DataHandler, T>::value,
                      "DataHandlerPrinter can only be used with classes "
                      "derived from DataHandler.");

      protected:
        T dataHandler;
        const std::string nameRegisterVariable;
        const std::string nameConstantVariable;
        const std::string nameDataVariable;

        std::string print1DArray(const int& start, const int& size,
                                 const std::string& nameVar) const;

      public:
        /**
         * \brief  constructor
         *
         * @param nameDataVariable
         * @param nameRegisterVariable
         * @param nameConstantVariable
         */

        DataHandlerPrinter(T d, const std::string& nameDataVariable = "in",
                           const std::string& nameRegisterVariable = "reg",
                           const std::string& nameConstantVariable = "cst")
            : dataHandler(d), nameRegisterVariable(nameRegisterVariable),
              nameDataVariable(nameDataVariable),
              nameConstantVariable(nameConstantVariable){};

        /// destructor
        virtual ~DataHandlerPrinter() = default;

        /// copy constructor
        DataHandlerPrinter(const DataHandlerPrinter<T>& other) = default;

        /**
         * \brief DataHandler
         *
         * // todo
         */
        std::string printDataAt(const std::type_info& type,
                                const size_t address,
                                const std::string& nameVar) const;

        std::string getTemplatedType() const;
    };

    template <class T>
    std::string DataHandlerPrinter<T>::printDataAt(
        const std::type_info& type, const size_t address,
        const std::string& nameVar) const
    {
#ifndef NDEBUG
        // Throw exception in case of invalid arguments.
        // méthod protect can't check this
        // dataHandler.checkAddressAndType(type, address);
#endif
        const std::type_info& templateType = dataHandler.getTemplateType();

        // Check if the operand need only one value
        if (type == templateType) {
            return std::string{" = " + nameVar + "[" + std::to_string(address) +
                               "];"};
        }

        // Else, we retreive all indexes of the global variable to initialize
        // the operand
        std::string operandInit{" = "};
        std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
        std::string regex{this->getTemplatedType()};
        regex.append("\\s*(const\\s*)?\\[([0-9]+)\\]");
        std::regex arrayType(regex);
        std::cmatch cm;
        size_t operandSize = 0;
        if (std::regex_match(typeName.c_str(), cm, arrayType)) {
            operandSize = std::atoi(cm[2].str().c_str());
        }
        operandInit += print1DArray(address, operandSize,nameVar);

        operandInit += ";";
        return operandInit;
    }

    template <class T>
    std::string DataHandlerPrinter<T>::getTemplatedType() const
    {
        return std::string{
            DEMANGLE_TYPEID_NAME(dataHandler.getTemplateType().name())};
    }

    template <typename T>
    std::string DataHandlerPrinter<T>::print1DArray(const int& start,
                                                    const int& size,
                                                    const std::string& nameVar) const
    {
        std::string operandInit{"{"};
        int end = start + size;
        for (size_t idx = start; idx < end; ++idx) {
            // idx.push_back(address + i);
            operandInit += (nameVar + "[" + std::to_string(idx) + "]");
            if (idx < (end - 1)) {
                operandInit += ", ";
            }
        }
        operandInit += "}";
        return operandInit;
    }

} // namespace Data

#endif // GEGELATI_DATAHANDLERPRINTER_H
#endif // CODE_GENERATION