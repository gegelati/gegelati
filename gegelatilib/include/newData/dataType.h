

/**
 * @file dataType.h
 * @brief Defines DataType, a descriptor for the shape, type, and source context of data.
 *
 * DataType is a **self-contained** descriptor that encapsulates:
 * 1. **Shape**: Rank (0=scalar, 1=1D, 2=2D) and dimensions.
 * 2. **Type**: `std::type_info` and element size (for pointer arithmetic).
 * 3. **Source Context**: For sub-views, stores the original source's shape and offset.
 *
 * This design allows DataView and DataValue to contain **only a data pointer and a DataType**,
 * while still supporting:
 * - Type safety (via elementType).
 * - Bounds safety (via sourceDimensions and sourceOffset).
 * - Sub-array extraction (via getSubView).
 */

#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <array>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <typeinfo>

namespace Data {

    /**
     * @brief Describes the shape, type, and source context of data.
     *
     * A DataType is a **complete descriptor** for data, including:
     * - **Shape**: Rank (0=scalar, 1=1D, 2=2D) and dimensions.
     * - **Type**: `std::type_info` of the element type and its size in bytes.
     * - **Source Context**: For sub-views, the original source's shape and offset.
     *
     * @note
     * - For **top-level data** (e.g., a DataValue), the source fields match the view's shape,
     *   and `sourceOffset = 0`.
     * - For **sub-views**, the source fields point to the **original source**, enabling correct
     *   address space calculations (e.g., `double[4]` from `double[8]` at address 1 → indices 1-4).
     * - `sourceOffset` is a **linear offset in elements** (not bytes). For 2D arrays, it is
     *   linearized in **row-major order** (offset = row * width + col).
     */
    struct DataType {
        // --- Shape of THIS view ---
        /// Number of dimensions (0=scalar, 1=1D array, 2=2D array).
        uint8_t rank = 0;

        /// Extent of each dimension. For 1D: [size, 0]. For 2D: [height, width].
        std::array<size_t, 2> dimensions{0, 0};

        // --- Type information ---
        /// Runtime type of the elements (e.g., `typeid(double)`).
        const std::type_info* elementType = nullptr;

        /// Size of each element in bytes (e.g., `sizeof(double)`).
        size_t elementSize = 0;

        // --- Source context (for sub-views) ---
        /// Rank of the original source (for top-level, equals `rank`).
        uint8_t sourceRank = 0;

        /// Dimensions of the original source (for top-level, equals `dimensions`).
        std::array<size_t, 2> sourceDimensions{0, 0};

        /// Linear offset (in **elements**) from the start of the source.
        /// For top-level, this is 0. For sub-views, this is the starting index in the source.
        size_t sourceOffset = 0;

        // --- Constructors ---

        /**
         * @brief Default constructor (invalid DataType).
         */
        DataType() = default;


        /**
         * @brief Constructs a DataType for a scalar.
         *
         * @tparam T The type of the scalar.
         */
        template <typename T>
        static DataType scalar() {
            DataType dt;
            dt.rank = 0;
            dt.dimensions = {1, 0};
            dt.elementType = &typeid(T);
            dt.elementSize = sizeof(T);
            dt.sourceRank = 0;
            dt.sourceDimensions = {1, 0};
            dt.sourceOffset = 0;
            return dt;
        }

        /**
         * @brief Constructs a DataType for a 1D array.
         *
         * @tparam T The element type of the array.
         * @param size The size of the array.
         */
        template <typename T>
        static DataType array1d(size_t size) {
            DataType dt;
            dt.rank = 1;
            dt.dimensions = {size, 0};
            dt.elementType = &typeid(T);
            dt.elementSize = sizeof(T);
            dt.sourceRank = 1;
            dt.sourceDimensions = {size, 0};
            dt.sourceOffset = 0;
            return dt;
        }

        /**
         * @brief Constructs a DataType for a 2D array (row-major).
         *
         * @tparam T The element type of the array.
         * @param rows The height (number of rows).
         * @param cols The width (number of columns).
         */
        template <typename T>
        static DataType array2d(size_t rows, size_t cols) {
            DataType dt;
            dt.rank = 2;
            dt.dimensions = {rows, cols};
            dt.elementType = &typeid(T);
            dt.elementSize = sizeof(T);
            dt.sourceRank = 2;
            dt.sourceDimensions = {rows, cols};
            dt.sourceOffset = 0;
            return dt;
        }

        template <typename T>
        static DataType from(const T (&)) {
            return scalar<T>();
        }

        template <typename T, size_t N>
        static DataType from(const T (&)[N]) {
            return array1d<T>(N);
        }

        template <typename T, size_t Rows, size_t Cols>
        static DataType from(const T (&)[Rows][Cols]) {
            return array2d<T>(Rows, Cols);
        }

        /**
         * @brief Constructs a DataType for a sub-view.
         *
         * @param viewShape The shape of the sub-view (e.g., [4] for a sub-array of [8]).
         * @param source The DataType of the source.
         * @param offset The linear offset (in elements) from the start of the source.
         */
        static DataType subView(DataType viewShape, const DataType& source, size_t offset) {
            DataType dt;
            dt.rank = viewShape.rank;
            dt.dimensions = viewShape.dimensions;
            dt.elementType = viewShape.elementType;
            dt.elementSize = viewShape.elementSize;
            dt.sourceRank = source.sourceRank;
            dt.sourceDimensions = source.sourceDimensions;
            dt.sourceOffset = offset;
            return dt;
        }

        // --- Helpers ---

        /**
         * @brief Returns the total number of elements in THIS view.
         */
        size_t totalElements() const noexcept {
            return dimensions[0] * (rank >= 2 ? dimensions[1] : 1);
        }

        /**
         * @brief Returns the total number of elements in the SOURCE.
         */
        size_t sourceTotalElements() const noexcept {
            return sourceDimensions[0] * (sourceRank >= 2 ? sourceDimensions[1] : 1);
        }

        /**
         * @brief Checks if this shape can fit in the source at a given offset.
         *
         * @param offset The linear offset (in elements) from the start of the source.
         * @return true if the shape fits at the given offset.
         */
        bool canFitIn(size_t offset) const noexcept {
            if (this->rank > this->sourceRank) {
                return false;
            }

            size_t start = sourceOffset + offset;

            if (rank == 0) {
                // Scalar: just check if the offset is within the source's total elements.
                return start < sourceTotalElements();
            }
            else if (rank == 1) {
                // 1D sub-view: must fit in the linearized source.
                return (start + dimensions[0]) <= sourceTotalElements();
            }
            else { // rank == 2
                // Convert linear start to (row, col) in the source.
                size_t startRow = start / sourceDimensions[1];
                size_t startCol = start % sourceDimensions[1];
                return (startRow + dimensions[0] <= sourceDimensions[0]) &&
                    (startCol + dimensions[1] <= sourceDimensions[1]);
            }
        }

        /**
         * @brief Computes the number of valid starting addresses for THIS shape in the source.
         *
         * @return The number of valid starting addresses, or 0 if the shape cannot be extracted.
         */
        size_t getAddressSpace() const noexcept {
            if (rank == 0) {
                return sourceTotalElements();
            }
            if (rank == 1) {
                if (sourceRank < 1) return 0;
                size_t available = sourceDimensions[0] - dimensions[0] + 1;
                return (available > sourceOffset) ? (available - sourceOffset) : 0;
            }
            if (rank == 2) {
                if (sourceRank < 2) return 0;
                size_t availableRows = sourceDimensions[0] - dimensions[0] + 1;
                size_t availableCols = sourceDimensions[1] - dimensions[1] + 1;
                return availableRows * availableCols;
            }
            return 0;
        }

        /**
         * @brief Compares two DataType objects for equality (ignores source context).
         *
         * Two DataTypes are equal if they have the same shape and type.
         * Source context is **not** compared (use compareWithSource() for that).
         */
        bool operator==(const DataType& other) const noexcept {
            return rank == other.rank &&
                   dimensions == other.dimensions &&
                   elementType == other.elementType &&
                   elementSize == other.elementSize;
        }

        /**
         * @brief Compares two DataType objects for inequality (ignores source context).
         */
        bool operator!=(const DataType& other) const noexcept {
            return !(*this == other);
        }

        /**
         * @brief Compares two DataType objects, including source context.
         */
        bool equalsWithSource(const DataType& other) const noexcept {
            return *this == other &&
                   sourceRank == other.sourceRank &&
                   sourceDimensions == other.sourceDimensions &&
                   sourceOffset == other.sourceOffset;
        }

        
        std::string toString() const {
            std::ostringstream oss;

            oss << "DataType{"
                << "rank=" << static_cast<int>(this->rank)
                << ", dimensions=[";

            for (size_t i = 0; i < this->rank; ++i) {
                if (i > 0) {
                    oss << ", ";
                }
                oss << this->dimensions[i];
            }

            oss << "]"
                << ", elementType=";

            if (this->elementType) {
                oss << this->elementType->name();
            } else {
                oss << "null";
            }

            oss << ", elementSize=" << this->elementSize
                << ", sourceRank=" << static_cast<int>(this->sourceRank)
                << ", sourceDimensions=[";

            for (size_t i = 0; i < this->sourceRank; ++i) {
                if (i > 0) {
                    oss << ", ";
                }
                oss << this->sourceDimensions[i];
            }

            oss << "]"
                << ", sourceOffset=" << this->sourceOffset
                << "}";

            return oss.str();
        }

    };

} // namespace Data

#endif