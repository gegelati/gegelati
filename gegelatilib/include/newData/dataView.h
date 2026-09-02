#ifndef DATA_VIEW_H
#define DATA_VIEW_H

#include <cstddef>
#include <stdexcept>
#include <typeinfo>
#include <sstream>
#include <string>
#include <iostream>

#include "newData/dataType.h"

namespace Data {

    /**
     * \brief Non-owning, read-only view of data with full metadata.
     *
     * A DataView consists of:
     * 1. A **pointer** to the first byte of the viewed data.
     * 2. A **DataType** describing the shape, type, and source context of the data.
     *
     * This design ensures that:
     * - **Type safety**: Typed accessors (getScalar<T>, getArray<T>) validate types at runtime.
     * - **Bounds safety**: Sub-array extraction (getSubView) validates that the requested
     *   shape fits in the source.
     * - **Source context**: The DataType carries the original source's shape and offset,
     *   enabling correct address space calculations.
     */
    class DataView {

    protected:
        /// Pointer to the first byte of the viewed data.
        const void* ptr = nullptr;

        /// Complete metadata for the viewed data (shape, type, source context).
        DataType type;

    public:
        /// \brief Default constructor (invalid view).
        DataView() = default;

        // --- Constructors ---

        /**
         * \brief Constructs a DataView with a pointer and DataType.
         *
         * \param[in] ptr Pointer to the first byte of the data.
         * \param[in] type The DataType describing the data.
         */
        explicit DataView(const void* ptr, DataType type)
            : ptr(ptr), type(std::move(type)) {}


        /**
         * \brief Constructs a DataView with a data of known type.
         *
         * Constructor automatically determine the dataType and get the given data as ptr.
         * 
         * \param[in] data data of known type.
         */
        template <typename T>
        DataView(const T& data)
            : ptr(static_cast<const void*>(&data)),
              type(DataType::from(data)) {}

        /**
         * \brief Constructs a DataView with a data of known type.
         *
         * Constructor automatically determine the dataType and get the given data as ptr.
         * 
         * \param[in] data data of known type.
         */
        template <typename T, size_t N>
        DataView(const T (&data)[N])
            : ptr(data),
              type(DataType::from(data)) {}

        // --- Accessors ---

        /**
         * \brief Returns the DataType describing this view.
         */
        const DataType& getType() const noexcept { return type; }

        /**
         * \brief Returns a pointer to the first byte of the viewed data.
         */
        const void* getData() const noexcept { return ptr; }

        /**
         * \brief Returns the shape of THIS view (rank + dimensions).
         */
        uint8_t getRank() const noexcept { return type.rank; }

        /**
         * \brief Returns the dimensions of THIS view.
         */
        const std::array<size_t, 2>& getDimensions() const noexcept { return type.dimensions; }

        /**
         * \brief Returns the element type of the viewed data.
         */
        const std::type_info& getElementType() const noexcept { return *type.elementType; }

        /**
         * \brief Returns the size of each element in bytes.
         */
        size_t getElementSize() const noexcept { return type.elementSize; }

        /**
         * \brief Returns the source's rank.
         */
        uint8_t getSourceRank() const noexcept { return type.sourceRank; }

        /**
         * \brief Returns the source's dimensions.
         */
        const std::array<size_t, 2>& getSourceDimensions() const noexcept { return type.sourceDimensions; }

        /**
         * \brief Returns the linear offset (in elements) in the source.
         */
        size_t getSourceOffset() const noexcept { return type.sourceOffset; }

        // --- Sub-View Extraction ---

        /**
         * \brief Checks whether a requested sub-view can be created from this view at a given address.
         *
         * This is the high-level compatibility check for `DataView`: it validates that the
         * requested descriptor fits into the current view's own layout at the provided offset,
         * while requiring the same element type.
         *
         * \param[in] requested The DataType of the requested sub-view (shape + element type).
         * \param[in] address The starting address in the current view's address space.
         * \return `true` if the sub-view can be safely created, otherwise `false`.
         */
        bool canFit(const DataType& requested, size_t address) const noexcept;

        /**
         * \brief Creates a sub-view of the data with a requested shape at a given address.
         *
         * This method **guarantees** that the returned view will only access valid elements
         * of the source data. For example:
         * - If the source is [8] and you request [4] at address 1, the sub-view will cover
         *   indices 1, 2, 3, 4 (not 0, 1, 2, 3).
         *
         * \param[in] requested The DataType of the requested sub-view (only shape is used).
         * \param[in] address The starting address in the address space.
         * \return A DataView of the requested sub-view.
         * @throws std::out_of_range If the requested shape does not fit at the given address.
         */
        DataView getSubView(DataType requested, size_t address) const;

        // --- Typed Accessors ---

        /**
         * controls a required rank and a type.
         * 
         * Throw if the method does not correspond to the requirement:
         * - ptr is not empty.
         * - rank is equal to requiredRank (0 for scalar, 1 for 1d array, 2 for 2d array)
         * - type is equivalent to register type T
         */
        void canBeAccess(const std::type_info& type, size_t requiredRank) const;

        /**
         * \brief Returns a reference to a scalar value of type T.
         *
         * @tparam T The type of the scalar.
         * \return A const reference to the scalar.
         * @throws std::runtime_error If the view is not a scalar or the type does not match.
         */
        template <typename T>
        const T& getScalar() const {
            this->canBeAccess(typeid(T), 0);
            return *static_cast<const T*>(this->ptr);
        }

        /**
         * \brief Returns a reference to a scalar value of type T at address specified.
         *
         * @tparam T The type of the scalar.
         * \return A const reference to the scalar.
         * @throws std::runtime_error If the view is not a scalar or the type does not match.
         */
        template <typename T>
        const T& getScalarAt(size_t address) const {
            Data::DataView subView = this->getSubView(Data::DataType::scalar<T>(), address);
            return subView.getScalar<T>();
        }

        /**
         * \brief Returns a pointer to the first element of a 1D or 2D array of type T.
         *
         * if its a non-contiguous 2D array, the pointer is copied.
         * 
         * @tparam T The element type of the array.
         * \return A const pointer to the first element of the array.
         * @throws std::runtime_error If the view is a scalar or the type does not match.
         */
        template <typename T>
        const T* getArray() const {
            using ValueType = std::remove_const_t<T>;

            this->canBeAccess(typeid(ValueType), 1);

            const ValueType* source =
                static_cast<const ValueType*>(this->ptr);

            const size_t rows = this->type.dimensions[0];
            const size_t cols = this->type.dimensions[1];
            const size_t stride = this->type.sourceDimensions[1];

            // Already contiguous: return the original memory.
            if (stride == cols || cols == 0) {
                return source;
            }

            // Strided sub-view: create a contiguous copy.
            ValueType* contiguous = new ValueType[rows * cols];

            for (size_t row = 0; row < rows; ++row) {
                for (size_t col = 0; col < cols; ++col) {
                    contiguous[row * cols + col] =
                        source[row * stride + col];
                }
            }

            return contiguous;
        }

        /**
         * \brief Checks if this view is valid (non-null).
         */
        explicit operator bool() const noexcept;

        /**
         * \brief Scale the location of the given address based on the required type
         * 
         * \param[in] required dataType requiring to be scaled
         * \param[in] address accessed in the current view
         */
        virtual size_t scaleLocation(const Data::DataType& required, const size_t address) const;

        /**
         * \brief override of toString method
         */
        virtual std::string toString() const;
    };

    inline std::ostream& operator<<(std::ostream& os, const DataView& view) {
        return os << view.toString();
    }

} // namespace Data

#endif // DATA_VIEW_H