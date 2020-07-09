#ifndef PRIMITIVE_TYPE_ARRAY_2D_H
#define PRIMITIVE_TYPE_ARRAY_2D_H

#include "data/primitiveTypeArray.h"

namespace Data {

    /**
     * \brief DataHandler for 2D arrays of primitive types.
     *
     * This specialization of the PrimitiveTypeArray template class additionally
     * provides the possibility to get data with the type:
     * - T[n][m]: with $n*m <=$ the size of the underlying PrimitiveTypeArray.
     */
    template <typename T>
    class PrimitiveTypeArray2D : public PrimitiveTypeArray<T>
    {
      protected:
        /// Number of columns of the 2D array.
        size_t width;

        /// Number of lines of the 2D array.
        size_t height;

        /**
         * \brief Utility function for the class.
         *
         * This method implements the getAddressSpace method, with an additional
         * argument that is used to return the number of array dimensions of the
         * requested type.
         *
         * \param[in] type The requested type_info.
         * \param[out] dim1 Size of the 1st dimension (if any).
         * \param[out] dim2 Size of the 2st dimension (if any).
         */
        size_t getAddressSpace(const std::type_info& type, size_t* dim1,
                               size_t* dim2) const;

      public:
        /**
         * \brief Constructor for the 2D array.
         *
         * The size of the underlying PrimitiveTypeArray will be $nbElements =
         * h*w$.
         *
         * \param[in] w The width of the 2D array.
         * \param[in] h The height of the 2D array.
         */
        PrimitiveTypeArray2D(const size_t w = 2, const size_t h = 4);

        // Inherited from DataHandler
        virtual size_t getAddressSpace(
            const std::type_info& type) const override;

        /// Inherited from DataHandler
        /*virtual UntypedSharedPtr getDataAt(const std::type_info& type,
                                           const size_t address) const override;*/
    };

    template <typename T>
    size_t PrimitiveTypeArray2D<T>::getAddressSpace(const std::type_info& type,
                                                    size_t* dim1,
                                                    size_t* dim2) const
    {
        size_t result = 0;
        // The parent function is not used to avoid performing 2 regex matches.
        if (type == typeid(T)) {
            result = this->nbElements;
        }

        // If the requested type is not provided by the underlying 1D array
        // Check if it is a 2D array
        if (result == 0) {

            // If the type is a 2D array of the primitive type
            // with a size (h*w) inferior to the container dimensions.
            std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
            std::string regex{DEMANGLE_TYPEID_NAME(typeid(T).name())};
            regex.append("\\s*(const\\s*)?\\[([0-9]+)\\](\\[([0-9]+)\\])?");
            std::regex arrayType(regex);
            std::cmatch cm;
            if (std::regex_match(typeName.c_str(), cm, arrayType)) {
                if (cm[2].matched && !cm[4].matched) {
                    // 1D array
                    int size = std::atoi(cm[2].str().c_str());
                    if (size <= this->nbElements) {
                        result = (this->nbElements - size + 1);
                        if (dim1 != nullptr) {
                            *dim1 = 1;
                        }
                    }
                }
                else if (cm[2].matched && cm[4].matched) {
                    // 2D array
                    int height = std::atoi(cm[2].str().c_str());
                    int width = std::atoi(cm[4].str().c_str());
                    if (height <= this->height && width <= this->width) {
                        result = (this->height - height + 1) *
                                 ((this->width - width + 1));
                        if (dim1 != nullptr) {
                            *dim1 = height;
                        }
                        if (dim2 != nullptr) {
                            *dim2 = width;
                        }
                    }
                }
            }
        }

        return result;
    }

    template <typename T>
    inline PrimitiveTypeArray2D<T>::PrimitiveTypeArray2D(const size_t w,
                                                         const size_t h)
        : PrimitiveTypeArray<T>(h * w), height(h), width(w)
    {
    }

    template <typename T>
    size_t PrimitiveTypeArray2D<T>::getAddressSpace(
        const std::type_info& type) const
    {
        // Use the utility function
        return this->getAddressSpace(type, nullptr, nullptr);
    }

    /* template <typename T>
    inline UntypedSharedPtr PrimitiveTypeArray2D<T>::getDataAt(
        const std::type_info& type, const size_t address) const
    {
#ifndef NDEBUG
        // Throw exception in case of invalid arguments.
        checkAddressAndType(type, address);
#endif

        if (type == typeid(T)) {
            UntypedSharedPtr result(
                &(this->data[address]),
                UntypedSharedPtr::emptyDestructor<const T>());
            return result;
        }

        // Else, the only other supported type is cstyle array (1D or 2D).

        size_t dim1 = 0;
        size_t dim2 = 0;
        size_t addressableSpace = this->getAddressSpace(type, &dim1, &dim2);

        if (dim2 == 0 && dim1 != 0) {
            // Allocate the array
            size_t arraySize = this->nbElements - dim1 + 1;
            T* array = new T[arraySize];

            // Copy its content
            for (size_t idx = 0; idx < arraySize; idx++) {
                array[idx] = this->data[address + idx];
            }

            // Create the UntypedSharedPtr
            UntypedSharedPtr result{
                std::make_shared<UntypedSharedPtr::Model<const T[]>>(array)};
            return result;
        }
        else if (dim2 != 0 && dim1 != 0) {
            // Allocate the array
            size_t arrayHeight = this->height - dim1 + 1;
            size_t arrayWidth = this->width - dim2 + 1;
            auto array = new T[arrayHeight][arrayWidth];

            // Copy its content
            for (size_t idx = 0; idx < arraySize; idx++) {
                array[idx] = this->data[address + idx];
            }

            // Create the UntypedSharedPtr
            UntypedSharedPtr result{
                std::make_shared<UntypedSharedPtr::Model<const T[][]>>((T**)array)};
            return result; 
        } 
    } */

}; // namespace Data
#endif
