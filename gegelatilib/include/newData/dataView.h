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
     * @brief Non-owning, read-only view of data with full metadata.
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
        /// Pointer to the first byte of the viewed data.
        const void* ptr = nullptr;

        /// Complete metadata for the viewed data (shape, type, source context).
        DataType type;

    public:
        /// @brief Default constructor (invalid view).
        DataView() = default;

        // --- Constructors ---

        /**
         * @brief Constructs a DataView with a pointer and DataType.
         *
         * @param ptr Pointer to the first byte of the data.
         * @param type The DataType describing the data.
         */
        DataView(const void* ptr, DataType type)
            : ptr(ptr), type(std::move(type)) {}


        template <typename T>
        DataView(const T (&data))
            : ptr(data),
            type(DataType::from(data)) {}

        // --- Accessors ---

        /**
         * @brief Returns the DataType describing this view.
         */
        const DataType& getType() const noexcept { return type; }

        /**
         * @brief Returns a pointer to the first byte of the viewed data.
         */
        const void* getData() const noexcept { return ptr; }

        /**
         * @brief Returns the shape of THIS view (rank + dimensions).
         */
        uint8_t getRank() const noexcept { return type.rank; }

        /**
         * @brief Returns the dimensions of THIS view.
         */
        const std::array<size_t, 2>& getDimensions() const noexcept { return type.dimensions; }

        /**
         * @brief Returns the element type of the viewed data.
         */
        const std::type_info& getElementType() const noexcept { return *type.elementType; }

        /**
         * @brief Returns the size of each element in bytes.
         */
        size_t getElementSize() const noexcept { return type.elementSize; }

        /**
         * @brief Returns the source's rank.
         */
        uint8_t getSourceRank() const noexcept { return type.sourceRank; }

        /**
         * @brief Returns the source's dimensions.
         */
        const std::array<size_t, 2>& getSourceDimensions() const noexcept { return type.sourceDimensions; }

        /**
         * @brief Returns the linear offset (in elements) in the source.
         */
        size_t getSourceOffset() const noexcept { return type.sourceOffset; }

        // --- Sub-View Extraction ---

        /**
         * @brief Checks whether a requested sub-view can be created from this view at a given address.
         *
         * This is the high-level compatibility check for `DataView`: it validates that the
         * requested descriptor fits into the current view's own layout at the provided offset,
         * while requiring the same element type.
         *
         * @param requested The DataType of the requested sub-view (shape + element type).
         * @param address The starting address in the current view's address space.
         * @return `true` if the sub-view can be safely created, otherwise `false`.
         */
        bool canFit(const DataType& requested, size_t address) const noexcept {
            if (requested.elementType == nullptr || this->type.elementType == nullptr) {
                return false;
            }
            if (requested.elementType != this->type.elementType) {
                return false;
            }
            return this->type.canFitIn(requested, address);
        }

        /**
         * @brief Creates a sub-view of the data with a requested shape at a given address.
         *
         * This method **guarantees** that the returned view will only access valid elements
         * of the source data. For example:
         * - If the source is [8] and you request [4] at address 1, the sub-view will cover
         *   indices 1, 2, 3, 4 (not 0, 1, 2, 3).
         *
         * @param requested The DataType of the requested sub-view (only shape is used).
         * @param address The starting address in the address space.
         * @return A DataView of the requested sub-view.
         * @throws std::out_of_range If the requested shape does not fit at the given address.
         */
        DataView getSubView(DataType requested, size_t address) const {
            if (!this->canFit(requested, address)) {
                throw std::out_of_range(
                    "DataView getSubView: Requested shape does not fit in the current view at the given address or type is wrong."
                );
            }
            // Compute the new linear offset in the source (in elements)
            size_t newOffset = this->type.sourceOffset;
            if (this->type.sourceRank >= 2 && requested.rank >= 2) {
                // For 2D, address is linearized in the address space:
                size_t row = (this->type.sourceOffset + address) / this->type.sourceDimensions[1];
                size_t col = (this->type.sourceOffset + address) % this->type.sourceDimensions[1];
                size_t newRowPos = (this->type.sourceOffset / this->type.sourceDimensions[1] + row);
                size_t newColPos = (this->type.sourceOffset % this->type.sourceDimensions[1] + col);
                newOffset = newRowPos * this->type.sourceDimensions[1] + newColPos;
            } else {
                // For 1D or scalar, address is linear
                newOffset = this->type.sourceOffset + address;
            }
            size_t newPtrOffset = address;
            if(this->type.sourceRank >= 2) {
                newPtrOffset += (this->type.sourceDimensions[0] - this->type.dimensions[0]) * (address / this->type.dimensions[1]);
            }
            const size_t byteOffset = newPtrOffset * this->type.elementSize;

            DataType subType = DataType::subView(
                requested,
                this->type,
                this->type.sourceOffset + newPtrOffset
            );

            return DataView(
                static_cast<const char*>(this->ptr) + byteOffset,
                std::move(subType)
            );
        }

        // --- Typed Accessors ---

        /**
         * controls a required rank and a type.
         * 
         * Throw if the method does not correspond to the requirement:
         * - ptr is not empty.
         * - rank is equal to requiredRank (0 for scalar, 1 for 1d array, 2 for 2d array)
         * - type is equivalent to register type T
         */
        void canBeAccess(const std::type_info& type, size_t requiredRank) const {
            if (!this->ptr) {
                throw std::runtime_error("Null DataView access.");
            }
            if (this->type.rank < requiredRank) {
                throw std::runtime_error(
                    "Cannot get required type data (rank: " +std::to_string(requiredRank) + ") from current DataView (rank: " +
                    std::to_string(this->type.rank) + ")."
                );
            }
            if (*this->type.elementType != type) {
                throw std::runtime_error(
                    "Type mismatch: expected " + std::string(this->type.elementType->name()) +
                    ", got " + std::string(type.name()) + "."
                );
            }
        }

        /**
         * @brief Returns a reference to a scalar value of type T.
         *
         * @tparam T The type of the scalar.
         * @return A const reference to the scalar.
         * @throws std::runtime_error If the view is not a scalar or the type does not match.
         */
        template <typename T>
        const T& getScalar() const {
            this->canBeAccess(typeid(T), 0);
            return *static_cast<const T*>(this->ptr);
        }

        /**
         * @brief Returns a reference to a scalar value of type T at address specified.
         *
         * @tparam T The type of the scalar.
         * @return A const reference to the scalar.
         * @throws std::runtime_error If the view is not a scalar or the type does not match.
         */
        template <typename T>
        const T& getScalarAt(size_t address) const {
            Data::DataView subView = this->getSubView(Data::DataType::scalar<T>(), address);
            return subView.getScalar<T>();
        }

        /**
         * @brief Returns a pointer to the first element of a 1D or 2D array of type T.
         *
         * if its a non-contiguous 2D array, the pointer is copied.
         * 
         * @tparam T The element type of the array.
         * @return A const pointer to the first element of the array.
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
         * @brief Checks if this view is valid (non-null).
         */
        explicit operator bool() const noexcept {
            return this->ptr != nullptr;
        }

        

        size_t scaleLocation(const size_t address, const Data::DataType& type) const
        {
            return address % this->type.totalElements();
            // Should then include a scale if type is array 1 or 2d 
        }


        std::string toString() const {
            std::ostringstream oss;

            oss << "DataView{"
                << "ptr=" << ptr
                << ",\n DataType=" << type.toString();

            return oss.str();
        }
    };
    

} // namespace Data

#endif // DATA_VIEW_H