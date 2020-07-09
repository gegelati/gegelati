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
    };

    template <typename T>
    inline PrimitiveTypeArray2D<T>::PrimitiveTypeArray2D(const size_t w,
                                                         const size_t h)
        : PrimitiveTypeArray<T>(h * w)
    {
    }
}; // namespace Data
#endif
