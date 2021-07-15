#ifdef CODE_GENERATION
#ifndef DATAHANDLERPRINTER_H
#define DATAHANDLERPRINTER_H

//#include "data/abstractDataHandlerPrinter.h"
#include "data/array2DWrapper.h"
#include "data/arrayWrapper.h"
#include "data/dataHandler.h"

namespace Data {
    /**
     * DataHandlerPrinter
     *
     * // todo
     *
     * @tparam T
     */
    // template <typename T>
    class DataHandlerPrinter //: public AbstractDataHandlerPrinter
    {
        //        static_assert(std::is_base_of<Data::DataHandler, T>::value,
        //                      "DataHandlerPrinter can only be used with
        //                      classes " "derived from DataHandler.");
      protected:
        const Data::DataHandler* const dataHandler;
        //        const std::string nameRegisterVariable;
        //        const std::string nameConstantVariable;
        //        const std::string nameDataVariable;

        /*virtual*/ std::string print1DArray(
            const size_t& start, const size_t& size,
            const std::string& nameVar) const; // override;

        /*virtual*/ std::string print2DArray(
            const size_t& start, const std::vector<size_t>& sourceTabSize,
            const std::vector<size_t>& generatedTabSize,
            const std::string& nameVar) const; // override;

      public:
        /**
         * \brief  constructor
         *
         * \param[in] dataHandler
         * \param[in] nameDataVariable
         * \param[in] nameRegisterVariable
         * \param[in] nameConstantVariable
         */

        DataHandlerPrinter(const Data::DataHandler* const dataHandler,
                           const std::string& nameDataVariable = "in",
                           const std::string& nameRegisterVariable = "reg",
                           const std::string& nameConstantVariable = "cst")
            : dataHandler{dataHandler}
              //            : AbstractDataHandlerPrinter{dataHandler,
              //            nameDataVariable,
              //                                         nameRegisterVariable,
              //                                         nameConstantVariable}
              {
                  //            if (typeid(T) != dataHandler->getTemplateType())
                  //            {
                  //                throw std::invalid_argument(
                  //                    "the template type of the
                  //                    DataHandlerPrinter is different "
                  //                    "from the template of the DataHandler
                  //                    given as parameter.");
                  //            }
              };

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
        /*virtual*/ std::string printDataAt(const std::type_info& type,
                                            const size_t address,
                                            const std::string& nameVar) const
            /*override*/;
        /**
         * \brief function used to retrieve the typename of the template of the
         * DataHandler
         *
         * \return template type of the DataHandler in a human readable format
         */
        /*virtual*/ std::string getTemplatedType() const /*override*/;

        std::vector<size_t> getOperandSizes(const std::type_info& type) const;
    };

    /*//    template <class T>
    //    std::string Data::DataHandlerPrinter<T>::printDataAt(
    //        const std::type_info& type, const size_t address,
    //        const std::string& nameVar) const
    //    {
    //        const std::type_info& templateType =
    dataHandler->getTemplateType();
    //        Data::ArrayWrapper<T>* ptrArray1D = nullptr;
    //        Data::Array2DWrapper<T>* ptrArray2D = nullptr;
    //
    //        // Check if the operand need only one value
    //        if (type == templateType) {
    //            return std::string{" = " + nameVar + "[" +
    std::to_string(address) +
    //                               "];"};
    //        }
    //        else if ((ptrArray2D = dynamic_cast<Data::Array2DWrapper<T>*>(
    //                      dataHandler)) != nullptr) {
    //
    //        }
    //        else if ((ptrArray1D = dynamic_cast<Data::ArrayWrapper<T>*>(
    //                      dataHandler)) != nullptr) {
    //        }
    //
    //        // Else, we retreive all indexes of the global variable to
    initialize
    //        // the operand for a 1D array
    //        std::string operandInit{"[] = "};
    //        std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
    //        std::string regex{this->getTemplatedType()};
    //        regex.append("\\s*(const\\s*)?\\[([0-9]+)\\]");
    //        std::regex arrayType(regex);
    //        std::cmatch cm;
    //        size_t operandSize = 0;
    //
    //        if (std::regex_match(typeName.c_str(), cm, arrayType)) {
    //
    //            operandSize = std::atoi(cm[2].str().c_str());
    //        }
    //        operandInit += print1DArray(address, operandSize, nameVar);
    //
    //        operandInit += ";";
    //        return operandInit;
    //    }
    //
    //    template <class T>
    //    std::string Data::DataHandlerPrinter<T>::getTemplatedType() const
    //    {
    //        return std::string{
    //            DEMANGLE_TYPEID_NAME(dataHandler->getTemplateType().name())};
    //    }
    //
    //    template <class T>
    //    std::string Data::DataHandlerPrinter<T>::print1DArray(
    //        const size_t& start, const size_t& size,
    //        const std::string& nameVar) const
    //    {
    //        std::string array{"{"};
    //        int end = start + size;
    //        for (size_t idx = start; idx < end; ++idx) {
    //            array += (nameVar + "[" + std::to_string(idx) + "]");
    //            if (idx < (end - 1)) {
    //                array += ", ";
    //            }
    //        }
    //        array += "}";
    //        return array;
    //    }
    //
    //    template <class T>
    //    std::string Data::DataHandlerPrinter<T>::print2DArray(
    //        const size_t& start, const std::pair<size_t, size_t>&
    sourceTabSize,
    //        const std::pair<size_t, size_t>& generatedTabSize,
    //        const std::string& nameVar)
    //    {
    //
    //        return std::__cxx11::string();
    //    }*/

} // namespace Data

#endif // DATAHANDLERPRINTER_H
#endif // CODE_GENERATION