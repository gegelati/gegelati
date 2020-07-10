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
    };

    template <typename T>
    inline PrimitiveTypeArray2D<T>::PrimitiveTypeArray2D(const size_t w,
                                                         const size_t h)
        : PrimitiveTypeArray<T>(h * w), height(h), width(w)
    {
    }

    template <typename T>
    inline size_t PrimitiveTypeArray2D<T>::getAddressSpace(
        const std::type_info& type) const
    {
        // Check if type is provided by underlying 1D array.
        size_t result = Data::PrimitiveTypeArray<T>::getAddressSpace(type);

        // If the requested type is not provided by the undetlying 1D array
        // Check if it is a 2D array
        if (result == 0) {

            // If the type is a 2D array of the primitive type
            // with a size (h*w) inferior to the container dimensions.
            std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
            std::string regex{DEMANGLE_TYPEID_NAME(typeid(T).name())};
            regex.append("\\s*(const\\s*)?\\[([0-9]+)\\]\\[([0-9]+)\\]");
            std::regex arrayType(regex);
            std::cmatch cm;
            if (std::regex_match(typeName.c_str(), cm, arrayType)) {
                int height = std::atoi(cm[2].str().c_str());
                int width = std::atoi(cm[3].str().c_str());
                if (height <= this->height && width <= this->width) {
                    return (this->height - height + 1) *
                           ((this->width - width + 1));
                }
            }
        }

        return result;
    }

}; // namespace Data
#endif
