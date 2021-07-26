#ifdef CODE_GENERATION
#ifndef DATAHANDLERPRINTER_H
#define DATAHANDLERPRINTER_H

#include "data/dataHandler.h"
#include "data/demangle.h"

namespace Data {
    /**
     * \brief Class used to generate the declaration of operands of a line for
     * the code gen.
     */
    class DataHandlerPrinter
    {
      protected:
        /// Pointer to the DataHandler that will be printed.
        const Data::DataHandler* const dataHandler;

      private:
        /// Default constructor deleted for its uselessness.
        DataHandlerPrinter() = delete;

      public:
        /**
         * \brief  Constructor for the DataHandlerPrinter
         *
         * \param[in] dataHandler const reference to the DataHandler that need
         * to be printed
         */
        DataHandlerPrinter(const Data::DataHandler* const dataHandler)
            : dataHandler{dataHandler} {};

        /// destructor
        virtual ~DataHandlerPrinter() = default;

        /// copy constructor (shallow)
        DataHandlerPrinter(const DataHandlerPrinter& other) = default;

        /**
         * \brief Function used to generate the declaration of an operand based
         * on its type.
         *
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
        std::string printDataAt(const std::type_info& type,
                                const size_t address,
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
         * \return template type of the DataHandler in a human readable format.
         */
        std::string getDemangleTemplateType() const;

        /**
         * \brief Function that return the size of each dimension of an operand.
         *
         * \param[in] type the std::type_info of the operand.
         * \return a std::vector containing the size of each dimension of the
         * operand of type type.
         */
        static std::vector<size_t> getOperandSizes(const std::type_info& type);
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
    //        std::string regex{this->getDemangleTemplateType()};
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
    //    std::string Data::DataHandlerPrinter<T>::getDemangleTemplateType() const
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