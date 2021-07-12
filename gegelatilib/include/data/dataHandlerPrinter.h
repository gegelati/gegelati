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
    //    template <typename T>
    class DataHandlerPrinter
    {
        //        static_assert(std::is_base_of<Data::DataHandler, T>::value,
        //                      "DataHandlerPrinter can only be used with
        //                      classes " "derived from DataHandler.");

      protected:
        const Data::DataHandler* const dataHandler;
        //        const std::string nameRegisterVariable;
        //        const std::string nameConstantVariable;
        //        const std::string nameDataVariable;

        std::string print1DArray(const size_t& start, const size_t& size,
                                 const std::string& nameVar) const;

//        std::string print2DArray(const size_t& start, const size_t& w,
//                                 const size_t& h, const std::string& nameVar);

      public:
        /**
         * \brief  constructor
         *
         * \param[in] dataHandler
         * @param[in] nameDataVariable
         * @param[in] nameRegisterVariable
         * @param[in] nameConstantVariable
         */

        DataHandlerPrinter(const Data::DataHandler* const dataHandler,
                           const std::string& nameDataVariable = "in",
                           const std::string& nameRegisterVariable = "reg",
                           const std::string& nameConstantVariable = "cst")
            : dataHandler{dataHandler}
              //            , nameRegisterVariable(nameRegisterVariable),
              //              nameDataVariable(nameDataVariable),
              //              nameConstantVariable(nameConstantVariable)
              {};

        /// destructor
        virtual ~DataHandlerPrinter() = default;

        /// copy constructor
        //        DataHandlerPrinter(const DataHandlerPrinter<T>& other) =
        //        default;
        DataHandlerPrinter(const DataHandlerPrinter& other) = default;

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

    //    template <class T>
    //    std::string DataHandlerPrinter<T>::printDataAt(
    //    std::string DataHandlerPrinter::printDataAt(
    //        const std::type_info& type, const size_t address,
    //        const std::string& nameVar) const
    //    {

    //        const std::type_info& templateType =
    //        dataHandler->getTemplateType();
    //
    //        // Check if the operand need only one value
    //        if (type == templateType) {
    //            return std::string{" = " + nameVar + "[" +
    //            std::to_string(address) +
    //                               "];"};
    //        }
    //
    //        // Else, we retreive all indexes of the global variable to
    //        initialize
    //        // the operand
    //        std::string operandInit{" = "};
    //        std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
    //        std::string regex{this->getTemplatedType()};
    //        regex.append("\\s*(const\\s*)?\\[([0-9]+)\\]");
    //        std::regex arrayType(regex);
    //        std::cmatch cm;
    //        size_t operandSize = 0;
    //        if (std::regex_match(typeName.c_str(), cm, arrayType)) {
    //            operandSize = std::atoi(cm[2].str().c_str());
    //        }
    //        operandInit += print1DArray(address, operandSize,nameVar);
    //
    //        operandInit += ";";
    //        return operandInit;
    //    }
    //
    ////    template <class T>
    ////    std::string DataHandlerPrinter<T>::getTemplatedType() const
    //    std::string DataHandlerPrinter::getTemplatedType() const
    //    {
    //        return std::string{
    //            DEMANGLE_TYPEID_NAME(dataHandler->getTemplateType().name())};
    //    }
    //
    ////    template <typename T>
    ////    std::string DataHandlerPrinter<T>::print1DArray(const int& start,
    //    std::string DataHandlerPrinter::print1DArray(const int& start,
    //                                                    const int& size,
    //                                                    const std::string&
    //                                                    nameVar) const
    //    {
    //        std::string operandInit{"{"};
    //        int end = start + size;
    //        for (size_t idx = start; idx < end; ++idx) {
    //            // idx.push_back(address + i);
    //            operandInit += (nameVar + "[" + std::to_string(idx) + "]");
    //            if (idx < (end - 1)) {
    //                operandInit += ", ";
    //            }
    //        }
    //        operandInit += "}";
    //        return operandInit;
    //    }

} // namespace Data

#endif // GEGELATI_DATAHANDLERPRINTER_H
#endif // CODE_GENERATION