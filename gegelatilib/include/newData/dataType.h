

/**
 * @file dataType.h
 * \brief Defines DataType, a descriptor for the shape, type, and source context of data.
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
#include <cstdint>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace Data {

    /**
     * \brief Descriptor of a typed view or value, including shape and source context.
     *
     * `DataType` is the metadata object carried by `DataView` and `DataValue`.
     * It answers three questions about the memory it describes:
     *
     * 1. What is the logical shape of the view? (`rank`, `dimensions`)
     * 2. What element type does the memory store? (`elementType`, `elementSize`)
     * 3. If this is a sub-view, where does it come from in the original source?
     *    (`sourceRank`, `sourceDimensions`, `sourceOffset`)
     *
     * The class is intentionally lightweight and type-erased: it stores runtime type
     * information and dimensional metadata, but it does not own the underlying buffer.
     * This allows consumers to validate operations such as typed reads, sub-view creation,
     * and row-major indexing without having to duplicate the full data layout in every class.
     *
     * ### Shape model
     * - Rank 0: scalar
     * - Rank 1: 1D array
     * - Rank 2: 2D array
     *
     * For 1D arrays, `dimensions = {size, 0}`. For 2D arrays, `dimensions = {rows, cols}`.
     * All linear indexing is done in row-major order.
     *
     * ### Source context
     * For top-level objects, `sourceRank`, `sourceDimensions`, and `sourceOffset` match the
     * same data as the current view. For sub-views, they still point to the original buffer,
     * which allows algorithms to compute a valid window inside the source without losing the
     * original coordinate system.
     *
     * A typical example:
     * - source: `double[8]`
     * - view: `double[4]` at offset 1
     * - the view describes a window covering elements 1, 2, 3, 4 of the source.
     *
     * @note `sourceOffset` is expressed in elements, not bytes, and for 2D arrays it follows
     * row-major linearization: offset = row * width + col.
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
         * \brief Default constructor (invalid DataType).
         */
        DataType() = default;


        /**
         * \brief Builds a descriptor for a scalar value.
         *
         * A scalar is represented as a rank-0 object whose logical size is one element.
         * The `dimensions` field is normalized to `{1, 0}` so that downstream code can
         * treat scalar and array metadata uniformly when needed.
         *
         * @tparam T The scalar element type.
         * \return A DataType describing a single value of type T.
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
         * \brief Builds a descriptor for a 1D array.
         *
         * The view shape is stored as `{size, 0}` because the implementation uses a fixed
         * two-element dimension array: the first slot is the primary extent and the second is
         * unused for rank-1 data.
         *
         * @tparam T The element type of the array.
         * \param[in] size Number of elements in the array.
         * \return A DataType describing a contiguous 1D view of T.
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
         * \brief Builds a descriptor for a 2D array in row-major layout.
         *
         * The element ordering is assumed to be row-major: for a 2D matrix with dimension
         * `rows x cols`, the linear index of `(row, col)` is `row * cols + col`.
         *
         * @tparam T The element type of the array.
         * \param[in] rows Number of rows.
         * \param[in] cols Number of columns.
         * \return A DataType describing a 2D view of T.
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

        /**
         * \brief Deduce the DataType of a scalar object.
         *
         * This overload is intended to be used with a single object value, such as:
         * `DataType::from(value)`.
         */
        template <typename T>
        static DataType from(const T (&)) {
            return scalar<T>();
        }

        /**
         * \brief Deduce the DataType of a 1D C array.
         *
         * Example: `int values[4]; DataType::from(values)` creates an array1d<int>(4).
         */
        template <typename T, size_t N>
        static DataType from(const T (&)[N]) {
            return array1d<T>(N);
        }

        /**
         * \brief Deduce the DataType of a 2D C array.
         *
         * Example: `double matrix[2][3]; DataType::from(matrix)` creates an array2d<double>(2, 3).
         */
        template <typename T, size_t Rows, size_t Cols>
        static DataType from(const T (&)[Rows][Cols]) {
            return array2d<T>(Rows, Cols);
        }

        /**
         * \brief Creates a descriptor for a sub-view extracted from a larger source.
         *
         * The sub-view keeps the requested shape in `dimensions`, but also preserves the
         * metadata of the original source in `sourceRank`, `sourceDimensions`, and
         * `sourceOffset`. This lets code reconstruct valid pointer arithmetic when a window
         * is extracted from a larger 1D or 2D buffer.
         *
         * \param[in] viewShape Shape of the sub-view being extracted.
         * \param[in] source Metadata of the original source buffer.
         * \param[in] offset Linear offset to the beginning of the sub-view inside the source,
         * expressed in elements.
         * \return A DataType descriptor for the sub-view.
         */
        static DataType subView(DataType viewShape, const DataType& source, size_t offset);

        // --- Helpers ---

        /**
         * \brief Returns the number of elements in the current view.
         *
         * For scalar views this is 1. For rank-1 arrays, it is the array length.
         * For rank-2 arrays, it is `rows * cols`.
         */
        size_t totalElements() const noexcept;

        /**
         * \brief Returns the number of elements in the original source buffer.
         */
        size_t sourceTotalElements() const noexcept;

        /**
         * \brief Checks whether a requested descriptor can fit inside this descriptor.
         *
         * This is the descriptor-level compatibility check: it verifies whether a requested
         * view can be placed at a given linear offset inside the current view's own memory
         * layout, without crossing the current view boundaries and without mixing element types.
         *
         * The check is intentionally self-contained and does not depend on any external source
         * provenance. The source metadata is preserved for bookkeeping, but the validity test is
         * performed against the current descriptor itself.
         *
         * \param[in] requested The descriptor to be placed inside this descriptor.
         * \param[in] offset Linear offset, in elements, relative to the start of this descriptor.
         * \return `true` if the requested view fits in the current descriptor, otherwise `false`.
         */
        bool canFitIn(const DataType& requested, size_t offset = 0) const noexcept;

        /**
         * \brief Compares two descriptors while ignoring source context.
         *
         * Two DataTypes are considered identical if they describe the same view shape and
         * the same element type/size. The provenance of the underlying source is intentionally
         * not part of this comparison.
         */
        bool operator==(const DataType& other) const noexcept;

        /**
         * \brief Compares two descriptors while ignoring source context.
         */
        bool operator!=(const DataType& other) const noexcept;

        /**
         * \brief Compares two descriptors including the source provenance metadata.
         *
         * This is stricter than `operator==`: two views with the same logical shape and type but
         * different source origins are not considered equal in this comparison.
         */
        bool equalsWithSource(const DataType& other) const noexcept;

        /**
         * \brief Renders the descriptor as a debug-friendly string.
         *
         * The output is primarily intended for diagnostics and testing; it prints both the
         * local view metadata and the preserved source metadata.
         */
        std::string toString() const;

    };

    inline std::ostream& operator<<(std::ostream& os, const DataType type) {
        return os << type.toString();
    }

} // namespace Data

#endif